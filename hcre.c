#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <locale.h>
#include <string.h>
#include "rules.h"


typedef struct RuleHash {
    // The Rule struct itself
    Rule *rule;

    // Info about where the rule came from
    // This makes it easier to track down rule errors and dupes
    char *source_file, *source_text;
    unsigned int source_line;

    // Used by HASH_* functions
    UT_hash_handle hh;
} RuleHash;


void free_hash(RuleHash *hash) {
    if (hash == NULL) { return; }
    if (hash->rule       ) { free_rule(hash->rule);   }
    if (hash->source_file) { free(hash->source_file); }
    if (hash->source_text) { free(hash->source_text); }
    memset(hash, 0, sizeof(RuleHash));
}


void usage(char *hcre) {
    printf("\n");
    printf("USAGE:  %s  rule_file  [rule_file  ...]\n", hcre);
    printf("\n");
    printf("Input words are read from STDIN, mangled words are written on STDOUT\n");
    printf("All rule files are unioned together and duplicate rules are removed\n");
    printf("For a cross product of rules, pipe this program into another instance of itself\n");
    printf("\n");
    printf("\n");
    printf("EXAMPLE:\n");
    printf("    %s  best64.rule  <  words.txt\n", hcre);
    printf("    some_process  |  %s  leetspeak.rule  combinator.rule\n", hcre);
    printf("\n");
}


int main(int argc, char **argv) {
    if (argc <= 1) { usage(argv[0]); return 0; }

    // Allows rule upper/lower/toggle to follow locale
    setlocale(LC_CTYPE, "");


    // Our hash structure's head node
    RuleHash *rules = NULL;

    // For iterating rules hash with HASH_ITER
    RuleHash *hash_temp = NULL;

    // Output for prase_rule()
    Rule *cur_rule = NULL;

    // Reading/writing to rules hash
    RuleHash *cur_hash = NULL;

    // getline() will read into this
    char   *line     = NULL;
    ssize_t line_len = 0;
    size_t  line_malloc_size = 0;

    unsigned int error_count = 0;
    unsigned int dupe_count  = 0;


    // Process each file of rules
    for (int file_num = 1; file_num < argc; file_num++) {

        char *file_name = argv[file_num];
        FILE *rule_file = fopen(file_name, "rb");

        // Make sure we successfully opened the rule file
        if (rule_file == NULL) {
            fprintf(stderr, "ERROR: Failed to open input file <%s>\n", file_name);
            return -1;
        }


        // Read each rule from the input file
        unsigned int line_num = 0;
        while ( !feof(rule_file) ) {
            line_len = getline(&line, &line_malloc_size, rule_file);

            // Stop processing file on error or end of input (we don't care which)
            if (line_len <= 0) { break; }
            line_num++;

            // Trim trailing newline by overwriting it will a NULL terminator
            while (line[line_len - 1] == '\n' || line[line_len - 1] == '\r') {
                line[--line_len] = 0;
            }

            // Skip empty lines and commented lines
            if (line_len ==  0 ) { continue; }
            if (line[0]  == '#') { continue; }


            #ifdef DEBUG_PARSING
            fprintf(stderr, "Before Parsing: %s (Length: %d)\n", line, line_len);
            #endif

            // Prase and validate the rule into cur_rule
            int rule_check = parse_rule(line, line_len, &cur_rule);
            if (rule_check < 0) {
                fprintf(
                    stderr, "File: <%s>; Line: <%u>; Rule: <%s>; Error: %s\n",
                    file_name, line_num, line, cur_rule->text
                );

                error_count++;
                continue;
            }

            #ifdef DEBUG_PARSING
            fprintf(stderr, "After  Parsing: %s (Length: %d)\n", cur_rule->text, cur_rule->length);
            fprintf(stderr, "\n");
            #endif


            // Check if the cleaned rule is a dupe
            HASH_FIND(hh, rules, cur_rule->text, cur_rule->length, hash_temp);
            if (hash_temp != NULL) {
                #ifdef DEBUG_DUPES
                fprintf(
                    stderr, "File: <%s>; Line: <%u>; Rule: <%s>; Duplicate of <%s> from <%s> line <%u>\n",
                    file_name, line_num, line,
                    hash_temp->source_text, hash_temp->source_file, hash_temp->source_line
                );
                #endif

                dupe_count++;
                continue;
            }


            // Make a new hash entry from the current rule
            cur_hash = (RuleHash *)calloc(1, sizeof(RuleHash));
            cur_hash->rule        = clone_rule(cur_rule);
            cur_hash->source_file = strdup(file_name);
            cur_hash->source_line = line_num;
            cur_hash->source_text = strdup(line);

            // If clone_rule() failed, we have a major problem
            if (!cur_hash->rule) {
                fprintf(stderr, "Failed to clone rule, aborting\n");
                return -1;
            }

            HASH_ADD_KEYPTR(hh, rules, cur_hash->rule->text, cur_hash->rule->length, cur_hash);
        }


        fclose(rule_file);
    }

    #ifdef DEBUG_STATS
    fprintf(
        stderr, "Loaded %u rules, skipped %u broken and %u duplicate rules\n",
        HASH_COUNT(rules), error_count, dupe_count
    );
    #endif



    // Our mangled text ends up here
    char rule_output[BLOCK_SIZE];

    // Statistics
    unsigned long int word_count   = 0;
    unsigned long int reject_count = 0;

    // Increase the output buffering
    if (setvbuf(stdout, NULL, _IOFBF, 1024 * 1024) != 0) {
        fprintf(stderr, "Failed to adjust stdout buffer size\n");
    }

    // Main processing loop, runs for each line of input
    while ( !feof(stdin) ) {
        line_len = getline(&line, &line_malloc_size, stdin);

        // Error or end of input
        if (line_len <= 0) { break; }

        // Trim trailing newline, skip blank lines
        line[--line_len] = 0;
        if (line_len == 0) { continue; }


        // Safely iterate the rules hash
        HASH_ITER(hh, rules, cur_hash, hash_temp) {
            cur_rule = cur_hash->rule;

            // Apply the rule operations
            int rule_rtn = apply_rule(cur_rule, line, line_len, rule_output);

            // Something broke?
            if (rule_rtn < 0) {
                if (rule_rtn == REJECTED) {
                    // Rejections are expected, they're okay
                    reject_count++;

                } else {
                    // We missed something in parsing and now our rule broke
                    // We can't "fix" the rule, so our only option is to remove it
                    // If you ever see this message, please contact the developer
                    fprintf(stderr,
                        "Input word <%s> broke rule <%s> from file <%s>, line <%u> (parsed as <%s>): %s\n",
                        line,
                        cur_hash->source_text, cur_hash->source_file, cur_hash->source_line, cur_rule->text,
                        rule_output
                    );

                    HASH_DEL(rules, cur_hash);
                    free_hash(cur_hash);
                }

                // Regardless if this was a rejection or error, we're not printing this word
                continue;
            }


            // In debug mode, include the rule itself with the output
            #ifdef DEBUG_OUTPUT
            fwrite(cur_rule->text, cur_rule->length, 1, stdout);
            fputc('\t', stdout);
            #endif

            // Output the mangled word and a newline
            rule_output[rule_rtn++] = '\n';
            fwrite(rule_output, rule_rtn, 1, stdout);
            word_count++;
        }
    }


    #ifdef DEBUG_STATS
    fprintf(stderr, "Created %lu words, rejected %lu\n", word_count, reject_count);
    #endif

    return 0;
}
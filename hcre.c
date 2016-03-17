#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "rules.h"
#include "uthash.h"


typedef struct {
    char  *text;
    size_t length;
    UT_hash_handle hh;
} Rule;


int main(int argc, char **argv) {
    if (argc <= 1) {
        // TODO: Usage guide goes here
        return 0;
    }


    // I would kill for a vector right now
    unsigned int rule_arr_len     =   0;
    unsigned int rule_arr_len_max = 128;

    Rule *rules     = NULL; // Our hash structure's head node
    Rule *cur_rule  = NULL; // We read into/from our hash using this
    Rule *rule_temp = NULL; // Temp object for safe HASH_ITER usage
    char rule_output[BLOCK_SIZE];

    // getline will read into this
    char   *line     = NULL;
    int     line_num = 0;
    ssize_t line_len = 0;
    size_t  line_malloc_size = 0;

    // For each input file of rules...
    for (int file_num = 1; file_num < argc; file_num++) {

        char *file_name = argv[file_num];
        FILE *rule_file = fopen(file_name, "rb");

        // Make sure we successfully opened the rule file
        if (rule_file == NULL) {
            fprintf(stderr, "ERROR: Failed to open input file '%s'\n", file_name);
            return -1;
        }


        while ( !feof(rule_file) ) {
            line_num++;
            line_len = getline(&line, &line_malloc_size, rule_file);

            // Trim trailing newline by overwriting it will a NULL terminator
            line[--line_len] = 0;

            if (line_len <   0 ) { break;    } // Error or end of input
            if (line_len ==  0 ) { continue; } // Skip blank lines
            if (line[0]  == '#') { continue; } // Skip commented lines


            // We've read a rule!
            // Step one: make sure it's not a duplicate
            HASH_FIND_STR(rules, line, cur_rule);
            if (cur_rule) {
                fprintf(stderr, "%s:%d - skipping duplicate rule '%s'\n", file_name, line_num, line);
                continue;
            }


            // Step two: rule verification
            // Attempt to run the rule against a dummy string
            // This lets us check for syntax errors, invalid positions, etc
            int rule_check = apply_rule(line, line_len, " ", 1, rule_output);

            if (rule_check < 0 && rule_check != REJECTED) {
                // On non-rejection failure, an error message is stored in rule_output
                fprintf(stderr, "%s:%d - rule '%s' has errors: %s\n", file_name, line_num, line, rule_output);
                continue;
            }


            // Step three: add the rule
            cur_rule = (Rule *) malloc(sizeof(Rule));
            cur_rule->text   = strdup(line);
            cur_rule->length = line_len;
            HASH_ADD_KEYPTR(hh, rules, cur_rule->text, cur_rule->length, cur_rule);
        }

        fclose(rule_file);
    }

    #ifdef DEBUG
    fprintf(stderr, "Loaded %d rules\n", HASH_COUNT(rules));
    #endif


    // For each line of input...
    while ( !feof(stdin) ) {
        line_len = getline(&line, &line_malloc_size, stdin);
        line[--line_len] = 0;

        if (line_len <   0 ) { break;    } // Error or end of input
        if (line_len ==  0 ) { continue; } // Skip blank lines

        // For each rule...
        HASH_ITER(hh, rules, cur_rule, rule_temp) {

            // Apply the rule operations
            int rule_rtn = apply_rule(cur_rule->text, cur_rule->length, line, line_len, rule_output);

            // Our initial test may not have caught all possible breaking scenarios
            // If something does break, display the rule, input line, and error
            // Then delete the rule from the hash and free its memory
            if (rule_rtn < 0 && rule_rtn != REJECTED) {
                fprintf(stderr, "Input '%s', rule '%s' has errors: %s\n", line, cur_rule->text, rule_output);

                HASH_DEL(rules, cur_rule);
                free(cur_rule->text);
                free(cur_rule);
                continue;
            }


            // In debug mode, include the rule itself with the output
            #ifdef DEBUG
            fputs(cur_rule->text, stdout);
            fputc('\t',           stdout);
            #endif

            // Output the mangled word and a newline
            fputs(rule_output, stdout);
            fputc('\n',        stdout);
        }
    }

    return 0;
}
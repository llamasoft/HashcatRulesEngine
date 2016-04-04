// =============================================================================
// Original Author: Jens Steube <jens.steube@gmail.com>
// Rewritten By:    llamasoft   <llamasoft@users.noreply.github.com>
// License: MIT
// =============================================================================

#include "rules.h"

// Increment rule position, return syntax error on premature end
#define NEXT_RULEPOS(rule_pos)             \
    do {                                   \
        if ( ++(rule_pos) == rule_len ) {  \
            errno = PREMATURE_END_OF_RULE; \
            break;                         \
        }                                  \
    } while (0)

// Read current rule position as positional value into out_var
#define NEXT_RPTOI(rule, rule_pos, out_var)          \
    do {                                             \
        (out_var) = conv_ctoi( (rule)[(rule_pos)] ); \
        if ( (out_var) == -1 ) {                     \
            errno = INVALID_POSITIONAL;              \
            break;                                   \
        }                                            \
    } while (0)


static inline bool class_num(char c)   { return( (c >= '0') && (c <= '9') ); }
static inline bool class_lower(char c) { return( (c >= 'a') && (c <= 'z') ); }
static inline bool class_upper(char c) { return( (c >= 'A') && (c <= 'Z') ); }
static inline bool class_alpha(char c) { return(class_lower(c) || class_upper(c)); }

// Single character to integer value
// 0 .. 9 =>  0 ..  9
// A .. Z => 10 .. 35
// a .. z => 36 .. 61
// else -1 (error)
static inline int conv_ctoi(char c)
{
    if (class_num(c)) {
        return ((int)(c - '0'));

    } else if (class_upper(c)) {
        return ((int)(c - 'A' + (char)10));

    } else if (class_lower(c)) {
        return ((int)(c - 'a' + (char)36));

    } else {
        return(-1);
    }
}

// NOTE: toggle/lower/upper/switch functions used to be macros
// To prevent breakage, the signatures haven't been changed
// This also means that they have no return values and do no safety checks
// The functions are only used internally, so it shouldn't be an issue

// Toggle a character uppercase/lowercase at a given offset
void mangle_toggle_at(char str[BLOCK_SIZE], int offset) {
    if ( class_alpha(str[offset]) ) {
        str[offset] ^= 0x20;
    }
}


// Convert a character at offset to lowercase
void mangle_lower_at(char str[BLOCK_SIZE], int offset) {
    if ( class_upper(str[offset]) ) {
        str[offset] ^= 0x20;
    }
}


// Convert a character at offset to uppercase
void mangle_upper_at(char str[BLOCK_SIZE], int offset) {
    if ( class_lower(str[offset]) ) {
        str[offset] ^= 0x20;
    }
}


// Swap the characters at offsets left and right
void mangle_switch(char str[BLOCK_SIZE], int left, int right) {
    char temp  = str[left];
    str[left]  = str[right];
    str[right] = temp;
}


// Convert to lower
int mangle_lower_all(char str[BLOCK_SIZE], int str_len)
{
    for (int pos = 0; pos < str_len; pos++) { mangle_lower_at(str, pos); }

    return(str_len);
}


// Convert to upper
int mangle_upper_all(char str[BLOCK_SIZE], int str_len)
{
    for (int pos = 0; pos < str_len; pos++) { mangle_upper_at(str, pos); }

    return(str_len);
}


// Toggle case
int mangle_toggle_all(char str[BLOCK_SIZE], int str_len)
{
    for (int pos = 0; pos < str_len; pos++) { mangle_toggle_at(str, pos); }

    return(str_len);
}


// Reverse a string
int mangle_reverse(char str[BLOCK_SIZE], int str_len)
{
    char temp[BLOCK_SIZE];
    memcpy(temp, str, str_len);

    for (int i = 0; i < str_len; i++) {
        str[i] = temp[str_len - i - 1];
    }

    return(str_len);
}


// Append a string to itself
int mangle_double(char str[BLOCK_SIZE], int str_len)
{
    if ((str_len * 2) >= BLOCK_SIZE) { return(str_len); }

    memcpy(&str[str_len], str, (size_t)str_len);
    return(str_len * 2);
}


// Append a string to itself N times
int mangle_double_times(char str[BLOCK_SIZE], int str_len, int times)
{
    if ((str_len * times) + str_len >= BLOCK_SIZE) { return(str_len); }

    int orig_len = str_len;

    for (int i = 0; i < times; i++) {
        memcpy(&str[str_len], str, orig_len);

        str_len += orig_len;
    }

    return(str_len);
}


// Append a string to itself backwards
int mangle_reflect(char str[BLOCK_SIZE], int str_len)
{
    if ((str_len * 2) >= BLOCK_SIZE) { return(str_len); }

    mangle_double(str, str_len);
    mangle_reverse(&str[str_len], str_len);

    return(str_len * 2);
}


// Rotates a string left one character
int mangle_rotate_left(char str[BLOCK_SIZE], int str_len)
{
    if (str_len < 2) { return(str_len); }

    // Save the first character
    char temp = str[0];

    // Shift everything left
    for (int str_pos = 0; str_pos < str_len - 1; str_pos++) {
        str[str_pos] = str[str_pos + 1];
    }

    // Put the first character at the end
    str[str_len - 1] = temp;

    return(str_len);
}


// Rotates a string right one character
int mangle_rotate_right(char str[BLOCK_SIZE], int str_len)
{
    if (str_len < 2) { return(str_len); }

    // Save the last character
    char temp = str[str_len - 1];

    // Shift everything right
    for (int str_pos = str_len - 1; str_pos > 0; str_pos--) {
        str[str_pos] = str[str_pos - 1];
    }

    // Place the last character at the front
    str[0] = temp;

    return(str_len);
}


// Appends a single character to a string
int mangle_append(char str[BLOCK_SIZE], int str_len, char c)
{
    if ((str_len + 1) >= BLOCK_SIZE) { return(str_len); }

    str[str_len] = c;

    return(str_len + 1);
}


// Prepends a single character to a string
int mangle_prepend(char str[BLOCK_SIZE], int str_len, char c)
{
    if ((str_len + 1) >= BLOCK_SIZE) { return(str_len); }

    str[str_len] = c;
    mangle_rotate_right(str, str_len + 1);

    return(str_len + 1);
}


// Deletes a single character at offset
int mangle_delete_at(char str[BLOCK_SIZE], int str_len, int offset)
{
    if (offset >= str_len || offset < 0) { return(str_len); }

    for (int str_pos = offset; str_pos < str_len - 1; str_pos++) {
        str[str_pos] = str[str_pos + 1];
    }

    return(str_len - 1);
}


// Replaces string with substr_len characters starting at offset
int mangle_extract(char str[BLOCK_SIZE], int str_len, int offset, int substr_len)
{
    if (offset >= str_len) { return(str_len); }

    // substr_len is too large, shorten it so it fits within this string
    if ((offset + substr_len) > str_len) { substr_len = str_len - offset; }

    for (int str_pos = 0; str_pos < substr_len; str_pos++) {
        str[str_pos] = str[offset + str_pos];
    }

    return(substr_len);
}


// Removes substr_len characters starting at offset
int mangle_omit(char str[BLOCK_SIZE], int str_len, int offset, int substr_len)
{
    if (offset >= str_len) { return(str_len); }

    // We know offset is within the string, shorten substr_len
    //   so that offset + substr_len stays within the string
    // This effectively skips the for loop and turns this into a truncate
    if ((offset + substr_len) > str_len) { substr_len = str_len - offset; }

    for (int str_pos = offset; str_pos < str_len - substr_len; str_pos++) {
        str[str_pos] = str[str_pos + substr_len];
    }

    return(str_len - substr_len);
}


// Inserts a single character at offset, shifting the result down
int mangle_insert(char str[BLOCK_SIZE], int str_len, int offset, char c)
{
    // If offset is beyond end of string, treat as an append
    // offset == str_len allowed, same as appending character
    if (offset > str_len) { offset = str_len; }
    if ((str_len + 1) >= BLOCK_SIZE) { return(str_len); }

    for (int str_pos = str_len - 1; str_pos > offset - 1; str_pos--) {
        str[str_pos + 1] = str[str_pos];
    }

    str[offset] = c;

    return(str_len + 1);
}


// Insert substr_len characters from mem starting at position mem_offset into position offset
// str[0 .. offset - 1] + mem[mem_offset .. mem_offset + substr_len] + str[offset .. str_len]
int mangle_insert_multi(
    char str[BLOCK_SIZE], int str_len, int str_offset,
    char mem[BLOCK_SIZE], int mem_len, int mem_offset,
    int substr_len
)
{
    if (str_offset > str_len) { str_offset = str_len; }
    if ((str_len + substr_len) >= BLOCK_SIZE) { return(str_len); }

    if (mem_offset >= mem_len) { return(str_len); }
    if ((mem_offset + substr_len) > mem_len) { substr_len = mem_len - mem_offset; }
    if (substr_len < 1) { return(str_len); }

    // Shift mem down mem_offset characters
    // This is the substring we will add to str
    //   mem[mem_offset .. mem_offset + substr_len]
    memcpy(mem, mem + mem_offset, mem_len - mem_offset);

    // Append the back half of str (after str_offset) to mem
    // This will become the back half of the result
    //   mem[mem_offset .. mem_offset + substr_len] + str[str_offset .. str_len]
    memcpy(mem + substr_len, str + str_offset, str_len - str_offset);

    // Insert our result to the correct place in str
    memcpy(str + str_offset, mem, str_len - str_offset + substr_len);

    return(str_len + substr_len);
}


// Replace a single character at offset
int mangle_overstrike(char str[BLOCK_SIZE], int str_len, int offset, char c)
{
    if (offset >= str_len || offset < 0) { return(str_len); }

    str[offset] = c;

    return(str_len);
}


// Remove everything after position offset
int mangle_truncate_at(char str[BLOCK_SIZE], int str_len, int offset)
{
    if (offset >= str_len || offset < 0) { return(str_len); }

    // Not explicitly required, just messing with str to suppress a gcc warning
    str[offset] = 0;

    return(offset);
}


// Replace all instances of oldc with newc
int mangle_replace(char str[BLOCK_SIZE], int str_len, char oldc, char newc)
{
    for (int str_pos = 0; str_pos < str_len; str_pos++) {
        if (str[str_pos] != oldc) { continue; }

        str[str_pos] = newc;
    }

    return(str_len);
}


// Remove all instances of c
int mangle_purgechar(char str[BLOCK_SIZE], int str_len, char c)
{
    int ret_len, str_pos;
    for (ret_len = 0, str_pos = 0; str_pos < str_len; str_pos++) {
        if (str[str_pos] == c) { continue; }

        str[ret_len] = str[str_pos];

        ret_len++;
    }

    return(ret_len);
}


// Duplicate the first substr_len characters, prepending them to the string
//   str = "Apple", substr_len = 3, result = "AppApple"
int mangle_dupeblock_prepend(char str[BLOCK_SIZE], int str_len, int substr_len)
{
    if (substr_len < 1) { return(str_len); }

    // If substr_len is too long, shorten it to the string's length
    // This effectively makes it a "dupe word" operation
    if (substr_len > str_len) { substr_len = str_len; }
    if ((str_len + substr_len) >= BLOCK_SIZE) { return(str_len); }

    for (int str_pos = str_len - 1; str_pos >= 0; str_pos--) {
        str[str_pos + substr_len] = str[str_pos];
    }

    return(str_len + substr_len);
}


// Duplicate the first substr_len characters, appending them to the string
//   str = "Apple", substr_len = 3, result = "AppleApp"
int mangle_dupeblock_append(char str[BLOCK_SIZE], int str_len, int substr_len)
{
    if (substr_len < 1) { return(str_len); }

    // If substr_len is too long, shorten it to the string's length
    // This effectively makes it a "dupe word" operation
    if (substr_len > str_len) { substr_len = str_len; }
    if ((str_len + substr_len) >= BLOCK_SIZE) { return(str_len); }

    memcpy(&str[str_len], str, substr_len);

    return(str_len + substr_len);
}


// Duplicate the character at offset substr_len times
int mangle_dupechar_at(char str[BLOCK_SIZE], int str_len, int offset, int substr_len)
{
    if (str_len == 0) { return(str_len); }
    if ((str_len + substr_len) >= BLOCK_SIZE) { return(str_len); }

    char c = str[offset];
    for (int i = 0; i < substr_len; i++) {
        str_len = mangle_insert(str, str_len, offset, c);
    }

    return(str_len);
}


// Duplicates every character
int mangle_dupechar(char str[BLOCK_SIZE], int str_len)
{
    if (str_len == 0) { return(str_len); }
    if ((str_len + str_len) >= BLOCK_SIZE) { return(str_len); }

    for (int str_pos = str_len - 1; str_pos > -1; str_pos--) {
        int new_pos = str_pos * 2;

        str[new_pos] = str[str_pos];

        str[new_pos + 1] = str[str_pos];
    }

    return(str_len * 2);
}


// Swap the characters at positions offset and offset2
int mangle_switch_at_check(char str[BLOCK_SIZE], int str_len, int offset, int offset2)
{
    if (offset  >= str_len) { return(str_len); }
    if (offset2 >= str_len) { return(str_len); }

    mangle_switch(str, offset, offset2);

    return(str_len);
}


// Swap the characters at positions offset and offset2, no safety checks
int mangle_switch_at(char str[BLOCK_SIZE], int str_len, int offset, int offset2)
{
    mangle_switch(str, offset, offset2);

    return(str_len);
}


// Left bit-shift the character at offset
int mangle_chr_shiftl(uint8_t str[BLOCK_SIZE], int str_len, int offset)
{
    if (offset >= str_len) { return(str_len); }

    str[offset] <<= 1;

    return(str_len);
}


// Right bit-shift the character at offset
int mangle_chr_shiftr(uint8_t str[BLOCK_SIZE], int str_len, int offset)
{
    if (offset >= str_len) { return(str_len); }

    str[offset] >>= 1;

    return(str_len);
}


// Increment the character at offset
int mangle_chr_incr(uint8_t str[BLOCK_SIZE], int str_len, int offset)
{
    if (offset >= str_len) { return(str_len); }

    str[offset] += 1;

    return(str_len);
}


// Decrement the character at offset
int mangle_chr_decr(uint8_t str[BLOCK_SIZE], int str_len, int offset)
{
    if (offset >= str_len) { return(str_len); }

    str[offset] -= 1;

    return(str_len);
}


// Convert a string to title case
int mangle_title(char str[BLOCK_SIZE], int str_len)
{
    int upper_next = 1;

    for (int pos = 0; pos < str_len; pos++) {
        if (str[pos] == ' ') {
            upper_next = 1;
            continue;
        }

        if (upper_next) {
            upper_next = 0;
            mangle_upper_at(str, pos);

        } else {
            mangle_lower_at(str, pos);
        }
    }

    return(str_len);
}



Rule *clone_rule(Rule *source) {
    if (source       == NULL) { return NULL; }
    if (source->text == NULL) { return NULL; }

    Rule *rtn = (Rule *)calloc(1, sizeof(Rule));
    if (rtn == NULL) {
        fprintf(stderr, "clone_rule() failed to calloc() a Rule of size %zu\n", sizeof(Rule));
        return NULL;
    }

    rtn->text = (char *)calloc(1, (source->length + 1) * sizeof(char));
    if (rtn->text == NULL) {
        fprintf(stderr, "clone_rule() failed to calloc() a string of length %d\n", source->length + 1);
        return NULL;
    }
    memcpy(rtn->text, source->text, source->length);

    rtn->length = source->length;
    return rtn;
}


void free_rule(Rule *rule) {
    if (rule == NULL) { return; }
    if (rule->text) { free(rule->text); }
    memset(rule, 0, sizeof(Rule));
}



// Doesn't actually run the rule, but checks it for validity and does some preprocessing
//   e.g. removing noops, validating positionals, parameter count checking
// In theory, this will make apply_rule be faster as it will require fewer validations
// It also assists in detecting duplicate rules (e.g. 'lu' == 'l:u')
int parse_rule(char *rule, int rule_len, Rule **output_rule) {
    if (rule == NULL) { return(INVALID_INPUT); }

    // Allocate a Rule if our user was lazy
    if ((*output_rule) == NULL) {
        (*output_rule) = (Rule *)calloc(1, sizeof(Rule));
    }

    // Our new rule is guaranteed to be no larger than the unparsed rule
    // Note: realloc(NULL, size) is the same as malloc(size)
    char *new_rule     = (char *)realloc((*output_rule)->text, (rule_len + 1) * sizeof(char));
    int   new_rule_len = 0;
    memset(new_rule, 0, rule_len + 1);

    int errno = 0;
    int mem_len = 0;
    int temp_int;

    // Our cursor positions for the rule
    int rule_pos = 0;
    for (rule_pos = 0; rule_pos < rule_len; rule_pos++) {
        // Always copy the operation
        new_rule[new_rule_len++] = rule[rule_pos];

        switch (rule[rule_pos]) {
            // Whitespace and no-ops are skipped
            case ' ':
            case '\t':
            case '\r':
            case RULE_OP_MANGLE_NOOP:
                // Un-copy the operation
                new_rule[new_rule_len--] = 0;
                break;

            // No parameters
            case RULE_OP_MANGLE_LREST:
            case RULE_OP_MANGLE_UREST:
            case RULE_OP_MANGLE_LREST_UFIRST:
            case RULE_OP_MANGLE_UREST_LFIRST:
            case RULE_OP_MANGLE_TREST:
            case RULE_OP_MANGLE_REVERSE:
            case RULE_OP_MANGLE_DUPEWORD:
            case RULE_OP_MANGLE_REFLECT:
            case RULE_OP_MANGLE_ROTATE_LEFT:
            case RULE_OP_MANGLE_ROTATE_RIGHT:
            case RULE_OP_MANGLE_DELETE_FIRST:
            case RULE_OP_MANGLE_DELETE_LAST:
            case RULE_OP_MANGLE_DUPECHAR_ALL:
            case RULE_OP_MANGLE_SWITCH_FIRST:
            case RULE_OP_MANGLE_SWITCH_LAST:
            case RULE_OP_MANGLE_TITLE:
                // Operation already copied, nothing to do
                break;

            // Integer
            case RULE_OP_MANGLE_TOGGLE_AT:
            case RULE_OP_MANGLE_DUPEWORD_TIMES:
            case RULE_OP_MANGLE_DELETE_AT:
            case RULE_OP_MANGLE_TRUNCATE_AT:
            case RULE_OP_MANGLE_DUPECHAR_FIRST:
            case RULE_OP_MANGLE_DUPECHAR_LAST:
            case RULE_OP_MANGLE_DUPEBLOCK_FIRST:
            case RULE_OP_MANGLE_DUPEBLOCK_LAST:
            case RULE_OP_MANGLE_CHR_SHIFTL:
            case RULE_OP_MANGLE_CHR_SHIFTR:
            case RULE_OP_MANGLE_CHR_INCR:
            case RULE_OP_MANGLE_CHR_DECR:
            case RULE_OP_MANGLE_REPLACE_NP1:
            case RULE_OP_MANGLE_REPLACE_NM1:
            case RULE_OP_REJECT_LESS:
            case RULE_OP_REJECT_GREATER:
                NEXT_RULEPOS(rule_pos);
                NEXT_RPTOI(rule, rule_pos, temp_int);
                new_rule[new_rule_len++] = rule[rule_pos];
                break;

            // Character
            case RULE_OP_MANGLE_APPEND:
            case RULE_OP_MANGLE_PREPEND:
            case RULE_OP_MANGLE_PURGECHAR:
            case RULE_OP_REJECT_CONTAIN:
            case RULE_OP_REJECT_NOT_CONTAIN:
            case RULE_OP_REJECT_EQUAL_FIRST:
            case RULE_OP_REJECT_EQUAL_LAST:
                NEXT_RULEPOS(rule_pos);
                new_rule[new_rule_len++] = rule[rule_pos];
                break;

            // Character + Character
            case RULE_OP_MANGLE_REPLACE:
                NEXT_RULEPOS(rule_pos);
                new_rule[new_rule_len++] = rule[rule_pos];

                NEXT_RULEPOS(rule_pos);
                new_rule[new_rule_len++] = rule[rule_pos];
                break;

            // Integer + Integer
            case RULE_OP_MANGLE_EXTRACT:
            case RULE_OP_MANGLE_OMIT:
            case RULE_OP_MANGLE_SWITCH_AT:
                NEXT_RULEPOS(rule_pos);
                NEXT_RPTOI(rule, rule_pos, temp_int);
                new_rule[new_rule_len++] = rule[rule_pos];

                NEXT_RULEPOS(rule_pos);
                NEXT_RPTOI(rule, rule_pos, temp_int);
                new_rule[new_rule_len++] = rule[rule_pos];
                break;

            // Integer + Character
            case RULE_OP_MANGLE_INSERT:
            case RULE_OP_MANGLE_OVERSTRIKE:
            case RULE_OP_REJECT_EQUAL_AT:
            case RULE_OP_REJECT_CONTAINS:
                NEXT_RULEPOS(rule_pos);
                NEXT_RPTOI(rule, rule_pos, temp_int);
                new_rule[new_rule_len++] = rule[rule_pos];

                NEXT_RULEPOS(rule_pos);
                new_rule[new_rule_len++] = rule[rule_pos];
                break;

            // Memory write
            case RULE_OP_MEMORIZE_WORD:
                mem_len = 1;
                break;

            // Memory read
            case RULE_OP_MANGLE_APPEND_MEMORY:
            case RULE_OP_MANGLE_PREPEND_MEMORY:
            case RULE_OP_REJECT_MEMORY:
                if (!mem_len) { errno = MEMORY_ERROR; }
                break;

            // Memory read + Integer + Integer + Integer
            case RULE_OP_MANGLE_EXTRACT_MEMORY:
                if (!mem_len) { errno = MEMORY_ERROR; break; }

                NEXT_RULEPOS(rule_pos);
                NEXT_RPTOI(rule, rule_pos, temp_int);
                new_rule[new_rule_len++] = rule[rule_pos];

                NEXT_RULEPOS(rule_pos);
                NEXT_RPTOI(rule, rule_pos, temp_int);
                new_rule[new_rule_len++] = rule[rule_pos];

                NEXT_RULEPOS(rule_pos);
                NEXT_RPTOI(rule, rule_pos, temp_int);
                new_rule[new_rule_len++] = rule[rule_pos];
                break;

            default:
                errno = UNKNOWN_RULE_OP;
                break;
        }

        if (errno != 0) { break; }
    }


    // Check for processing errors and create an error message
    if (errno != 0) {
        // We use asprintf() to dynamically allocate a buffer for our error message
        // We don't need our old rule buffer any more because asprintf will create a new one
        free(new_rule);

        if (errno == PREMATURE_END_OF_RULE) {
            new_rule_len = asprintf(&new_rule,
                "premature end of rule, expected char or positional value"
            );

        } else if (errno == UNKNOWN_RULE_OP) {
            new_rule_len = asprintf(&new_rule,
                "'%c' (offset %d) is not a valid operation",
                rule[rule_pos], rule_pos
            );

        } else if (errno == INVALID_POSITIONAL) {
            new_rule_len = asprintf(&new_rule,
                "'%c' (offset %d) is not a valid position or length value",
                rule[rule_pos], rule_pos
            );

        } else if (errno == MEMORY_ERROR) {
            new_rule_len = asprintf(&new_rule,
                "'%c' (offset %d) cannot be used before memorize operation",
                rule[rule_pos], rule_pos
            );

        } else {
            new_rule_len = asprintf(&new_rule,
                "unknown error %d at operation '%c' (offset %d)",
                errno, rule[rule_pos], rule_pos
            );
            errno = UNKNOWN_ERROR;
        }
    }


    new_rule[new_rule_len] = 0;
    (*output_rule)->text   = new_rule;
    (*output_rule)->length = new_rule_len;
    return (errno < 0 ? errno : new_rule_len);
}



int apply_rule(Rule *input_rule, char *input_word, int input_len, char out[BLOCK_SIZE])
{
    if (input_rule       == NULL) { return(INVALID_INPUT); }
    if (input_rule->text == NULL) { return(INVALID_INPUT); }
    // Rule length not checked, noop rules are valid but empty
    char *rule     = input_rule->text;
    int   rule_len = input_rule->length;

    if (input_word == NULL) { return(INVALID_INPUT); }
    if (input_len  <     1) { return(INVALID_INPUT); }

    int  mem_len = -1;
    char mem[BLOCK_SIZE];

    int out_len = (input_len < BLOCK_SIZE ? input_len : BLOCK_SIZE - 1);
    memcpy(out, input_word, out_len);


    // Operation parameters
    int errno, rule_pos;
    for (rule_pos = 0, errno = 0; rule_pos < rule_len && !errno; rule_pos++) {
        switch (rule[rule_pos]) {
            case ' ':
            case '\t':
            case '\r':
            case RULE_OP_MANGLE_NOOP: {
                // After validation, noops shouldn't exist in the rule
                // Just in case though, we'll skip them
                break;
            }
            case RULE_OP_MANGLE_LREST: {
                mangle_lower_all(out, out_len);
                break;
            }
            case RULE_OP_MANGLE_UREST: {
                mangle_upper_all(out, out_len);
                break;
            }
            case RULE_OP_MANGLE_LREST_UFIRST: {
                mangle_lower_all(&out[1], out_len - 1);
                if (out_len > 0) { mangle_upper_at(out, 0); }
                break;
            }
            case RULE_OP_MANGLE_UREST_LFIRST: {
                mangle_upper_all(&out[1], out_len - 1);
                if (out_len > 0) { mangle_lower_at(out, 0); }
                break;
            }
            case RULE_OP_MANGLE_TREST: {
                mangle_toggle_all(out, out_len);
                break;
            }
            case RULE_OP_MANGLE_TOGGLE_AT: {
                int offset = conv_ctoi(rule[++rule_pos]);

                if (out_len > offset) { mangle_toggle_at(out, offset); }
                break;
            }
            case RULE_OP_MANGLE_REVERSE: {
                mangle_reverse(out, out_len);
                break;
            }
            case RULE_OP_MANGLE_DUPEWORD: {
                out_len = mangle_double(out, out_len);
                break;
            }
            case RULE_OP_MANGLE_DUPEWORD_TIMES: {
                int times = conv_ctoi(rule[++rule_pos]);

                out_len = mangle_double_times(out, out_len, times);
                break;
            }
            case RULE_OP_MANGLE_REFLECT: {
                out_len = mangle_reflect(out, out_len);
                break;
            }
            case RULE_OP_MANGLE_ROTATE_LEFT: {
                mangle_rotate_left(out, out_len);
                break;
            }
            case RULE_OP_MANGLE_ROTATE_RIGHT: {
                mangle_rotate_right(out, out_len);
                break;
            }
            case RULE_OP_MANGLE_APPEND: {
                char chr = rule[++rule_pos];

                out_len = mangle_append(out, out_len, chr);
                break;
            }
            case RULE_OP_MANGLE_PREPEND: {
                char chr = rule[++rule_pos];

                out_len = mangle_prepend(out, out_len, chr);
                break;
            }
            case RULE_OP_MANGLE_DELETE_FIRST: {
                out_len = mangle_delete_at(out, out_len, 0);
                break;
            }
            case RULE_OP_MANGLE_DELETE_LAST: {
                out_len = mangle_delete_at(out, out_len, out_len - 1);
                break;
            }
            case RULE_OP_MANGLE_DELETE_AT: {
                int offset = conv_ctoi(rule[++rule_pos]);

                out_len = mangle_delete_at(out, out_len, offset);
                break;
            }
            case RULE_OP_MANGLE_EXTRACT: {
                int offset     = conv_ctoi(rule[++rule_pos]);
                int substr_len = conv_ctoi(rule[++rule_pos]);

                out_len = mangle_extract(out, out_len, offset, substr_len);
                break;
            }
            case RULE_OP_MANGLE_OMIT: {
                int offset     = conv_ctoi(rule[++rule_pos]);
                int substr_len = conv_ctoi(rule[++rule_pos]);

                out_len = mangle_omit(out, out_len, offset, substr_len);
                break;
            }
            case RULE_OP_MANGLE_INSERT: {
                int  offset = conv_ctoi(rule[++rule_pos]);
                char chr    = rule[++rule_pos];

                out_len = mangle_insert(out, out_len, offset, chr);
                break;
            }
            case RULE_OP_MANGLE_OVERSTRIKE: {
                int  offset = conv_ctoi(rule[++rule_pos]);
                char chr    = rule[++rule_pos];

                mangle_overstrike(out, out_len, offset, chr);
                break;
            }
            case RULE_OP_MANGLE_TRUNCATE_AT: {
                int offset = conv_ctoi(rule[++rule_pos]);

                out_len = mangle_truncate_at(out, out_len, offset);
                break;
            }
            case RULE_OP_MANGLE_REPLACE: {
                char search  = rule[++rule_pos];
                char replace = rule[++rule_pos];

                mangle_replace(out, out_len, search, replace);
                break;
            }
            case RULE_OP_MANGLE_PURGECHAR: {
                char search = rule[++rule_pos];

                out_len = mangle_purgechar(out, out_len, search);
                break;
            }
            case RULE_OP_MANGLE_DUPECHAR_FIRST: {
                int substr_len = conv_ctoi(rule[++rule_pos]);

                out_len = mangle_dupechar_at(out, out_len, 0, substr_len);
                break;
            }
            case RULE_OP_MANGLE_DUPECHAR_LAST: {
                int substr_len = conv_ctoi(rule[++rule_pos]);

                out_len = mangle_dupechar_at(out, out_len, out_len - 1, substr_len);
                break;
            }
            case RULE_OP_MANGLE_DUPECHAR_ALL: {
                out_len = mangle_dupechar(out, out_len);
                break;
            }
            case RULE_OP_MANGLE_DUPEBLOCK_FIRST: {
                int substr_len = conv_ctoi(rule[++rule_pos]);

                out_len = mangle_dupeblock_prepend(out, out_len, substr_len);
                break;
            }
            case RULE_OP_MANGLE_DUPEBLOCK_LAST: {
                int substr_len = conv_ctoi(rule[++rule_pos]);

                out_len = mangle_dupeblock_append(out, out_len, substr_len);
                break;
            }
            case RULE_OP_MANGLE_SWITCH_FIRST: {
                if (out_len > 2) { mangle_switch_at(out, out_len, 0, 1); }
                break;
            }
            case RULE_OP_MANGLE_SWITCH_LAST: {
                if (out_len > 2) { mangle_switch_at(out, out_len, out_len - 1, out_len - 2); }
                break;
            }
            case RULE_OP_MANGLE_SWITCH_AT: {
                int offset1 = conv_ctoi(rule[++rule_pos]);
                int offset2 = conv_ctoi(rule[++rule_pos]);

                mangle_switch_at_check(out, out_len, offset1, offset2);
                break;
            }
            case RULE_OP_MANGLE_CHR_SHIFTL: {
                int offset = conv_ctoi(rule[++rule_pos]);

                mangle_chr_shiftl((uint8_t *) out, out_len, offset);
                break;
            }
            case RULE_OP_MANGLE_CHR_SHIFTR: {
                int offset = conv_ctoi(rule[++rule_pos]);

                mangle_chr_shiftr((uint8_t *) out, out_len, offset);
                break;
            }
            case RULE_OP_MANGLE_CHR_INCR: {
                int offset = conv_ctoi(rule[++rule_pos]);

                mangle_chr_incr((uint8_t *) out, out_len, offset);
                break;
            }
            case RULE_OP_MANGLE_CHR_DECR: {
                int offset = conv_ctoi(rule[++rule_pos]);

                mangle_chr_decr((uint8_t *) out, out_len, offset);
                break;
            }
            case RULE_OP_MANGLE_REPLACE_NP1: {
                int offset = conv_ctoi(rule[++rule_pos]);

                if ((offset + 1) < out_len) {
                    mangle_overstrike(out, out_len, offset, out[offset + 1]);
                }
                break;
            }
            case RULE_OP_MANGLE_REPLACE_NM1: {
                int offset = conv_ctoi(rule[++rule_pos]);

                if (offset >= 1 && offset < out_len) {
                    mangle_overstrike(out, out_len, offset, out[offset - 1]);
                }
                break;
            }
            case RULE_OP_MANGLE_TITLE: {
                mangle_title(out, out_len);
                break;
            }
            case RULE_OP_MANGLE_EXTRACT_MEMORY: {
                if (mem_len < 0) { errno = MEMORY_ERROR; break; }

                int mem_offset = conv_ctoi(rule[++rule_pos]);
                int substr_len = conv_ctoi(rule[++rule_pos]);
                int out_offset = conv_ctoi(rule[++rule_pos]);

                // If this results in a negative value, the memory was such that this output becomes rejected
                out_len = mangle_insert_multi(out, out_len, out_offset, mem, mem_len, mem_offset, substr_len);
                break;
            }
            case RULE_OP_MANGLE_APPEND_MEMORY: {
                if (mem_len < 0) { errno = MEMORY_ERROR; break; }
                if ((out_len + mem_len) >= BLOCK_SIZE) { break; }

                memcpy(out + out_len, mem, mem_len);
                out_len += mem_len;
                break;
            }
            case RULE_OP_MANGLE_PREPEND_MEMORY: {
                if (mem_len < 0) { errno = MEMORY_ERROR; break; }
                if ((out_len + mem_len) >= BLOCK_SIZE) { break; }

                // Use the unused portion of mem as a buffer area
                // We know it fits because (mem_len + out_len <= BLOCK_SIZE)
                memcpy(mem + mem_len, out, out_len);
                out_len += mem_len;
                memcpy(out, mem, out_len);
                break;
            }
            case RULE_OP_MEMORIZE_WORD: {
                memcpy(mem, out, out_len);
                mem_len = out_len;
                break;
            }
            case RULE_OP_REJECT_LESS: {
                int target_len = conv_ctoi(rule[++rule_pos]);

                if ( !(out_len <= target_len) ) { errno = REJECTED; }
                break;
            }
            case RULE_OP_REJECT_GREATER: {
                int target_len = conv_ctoi(rule[++rule_pos]);

                if ( !(out_len >= target_len) ) { errno = REJECTED; }
                break;
            }
            case RULE_OP_REJECT_CONTAIN: {
                char chr = rule[++rule_pos];

                if (memchr(out, chr, out_len) != NULL) { errno = REJECTED; }
                break;
            }
            case RULE_OP_REJECT_NOT_CONTAIN: {
                char chr = rule[++rule_pos];

                if (memchr(out, chr, out_len) == NULL) { errno = REJECTED; }
                break;
            }
            case RULE_OP_REJECT_EQUAL_FIRST: {
                char chr = rule[++rule_pos];

                if (out_len < 1 || out[0] != chr) { errno = REJECTED; }
                break;
            }
            case RULE_OP_REJECT_EQUAL_LAST: {
                char chr = rule[++rule_pos];

                if (out_len < 1 || out[out_len - 1] != chr) { errno = REJECTED; }
                break;
            }
            case RULE_OP_REJECT_EQUAL_AT: {
                int  offset = conv_ctoi(rule[++rule_pos]);
                char chr    = rule[++rule_pos];

                if (offset >= out_len || out[offset] != chr) { errno = REJECTED; }
                break;
            }
            case RULE_OP_REJECT_CONTAINS: {
                int  min_count = conv_ctoi(rule[++rule_pos]);
                char chr       = rule[++rule_pos];

                if (min_count > out_len) { errno = REJECTED; }

                int count = 0;
                for (int pos = 0; pos < out_len; pos++) {
                    if (out[pos] == chr) { count++; }
                    if (count >= min_count) { break; }
                }

                if (count < min_count) { errno = REJECTED; }
                break;
            }
            case RULE_OP_REJECT_MEMORY: {
                if (mem_len < 0) { errno = MEMORY_ERROR; break; }
                if (out_len == mem_len && memcmp(out, mem, out_len) == 0) { errno = REJECTED; }
                break;
            }
            default:
                errno = UNKNOWN_RULE_OP;
                break;
        }
    }

    if (errno != 0) { return(errno); }

    // Add the null terminator and null extra bytes (just in case)
    memset(out + out_len, 0, BLOCK_SIZE - out_len);
    return(out_len);
}
// =============================================================================
// Original Author: Jens Steube <jens.steube@gmail.com>
// Rewritten By:    llamasoft   <llamasoft@users.noreply.github.com>
// License: MIT
// =============================================================================

#ifndef RULES_H
#define RULES_H

#if __STDC_VERSION__ < 199901L
    #error C99 compatibility is required
#endif

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#include "uthash.h"

// Input is truncated to BLOCK_SIZE - 1 bytes
// Rules which push the output over BLOCK_SIZE - 1 are skipped
#ifndef BLOCK_SIZE
    #define BLOCK_SIZE 64
#endif


typedef struct Rule {
    char  *text;
    size_t length;
} Rule;


// Makes a copy of a rule and returns a pointer to the new copy
Rule *clone_rule(Rule *source);

// Frees the members of a rule structure
void free_rule(Rule *rule);

// Parses a rule into a Rule struct
// The rule is checked for validity and all no-ops are removed
int parse_rule(char *rule_text, int rule_text_length, Rule **output_rule);

// Applies a rule to an input word and saves the output to output_word
// If a rule operation would cause the length to extend beyond BLOCK_SIZE, the operation is skipped
// Do not pass a hand-crafted rule struct into this function, run it through parse_rule() first
int apply_rule(Rule *rule_to_apply, char *input_word, int input_len, char output_word[BLOCK_SIZE]);



enum RULE_RC {
    INVALID_INPUT = -99,
    PREMATURE_END_OF_RULE,
    UNKNOWN_RULE_OP,
    INVALID_POSITIONAL,
    MEMORY_ERROR,
    REJECTED,
    UNKNOWN_ERROR
};


#define RULE_OP_MANGLE_NOOP             ':'
#define RULE_OP_MANGLE_LREST            'l'
#define RULE_OP_MANGLE_UREST            'u'
#define RULE_OP_MANGLE_LREST_UFIRST     'c'
#define RULE_OP_MANGLE_UREST_LFIRST     'C'
#define RULE_OP_MANGLE_TREST            't'
#define RULE_OP_MANGLE_TOGGLE_AT        'T'
#define RULE_OP_MANGLE_REVERSE          'r'
#define RULE_OP_MANGLE_DUPEWORD         'd'
#define RULE_OP_MANGLE_DUPEWORD_TIMES   'p'
#define RULE_OP_MANGLE_REFLECT          'f'
#define RULE_OP_MANGLE_ROTATE_LEFT      '{'
#define RULE_OP_MANGLE_ROTATE_RIGHT     '}'
#define RULE_OP_MANGLE_APPEND           '$'
#define RULE_OP_MANGLE_PREPEND          '^'
#define RULE_OP_MANGLE_DELETE_FIRST     '['
#define RULE_OP_MANGLE_DELETE_LAST      ']'
#define RULE_OP_MANGLE_DELETE_AT        'D'
#define RULE_OP_MANGLE_EXTRACT          'x'
#define RULE_OP_MANGLE_OMIT             'O'
#define RULE_OP_MANGLE_INSERT           'i'
#define RULE_OP_MANGLE_OVERSTRIKE       'o'
#define RULE_OP_MANGLE_TRUNCATE_AT      '\''
#define RULE_OP_MANGLE_REPLACE          's'
#define RULE_OP_MANGLE_PURGECHAR        '@'
#define RULE_OP_MANGLE_DUPECHAR_FIRST   'z'
#define RULE_OP_MANGLE_DUPECHAR_LAST    'Z'
#define RULE_OP_MANGLE_DUPECHAR_ALL     'q'
#define RULE_OP_MANGLE_EXTRACT_MEMORY   'X'
#define RULE_OP_MANGLE_APPEND_MEMORY    '4'
#define RULE_OP_MANGLE_PREPEND_MEMORY   '6'

#define RULE_OP_MEMORIZE_WORD           'M'

#define RULE_OP_REJECT_LESS             '<'
#define RULE_OP_REJECT_GREATER          '>'
#define RULE_OP_REJECT_CONTAIN          '!'
#define RULE_OP_REJECT_NOT_CONTAIN      '/'
#define RULE_OP_REJECT_EQUAL_FIRST      '('
#define RULE_OP_REJECT_EQUAL_LAST       ')'
#define RULE_OP_REJECT_EQUAL_AT         '='
#define RULE_OP_REJECT_CONTAINS         '%'
#define RULE_OP_REJECT_MEMORY           'Q'

#define RULE_OP_MANGLE_SWITCH_FIRST     'k'
#define RULE_OP_MANGLE_SWITCH_LAST      'K'
#define RULE_OP_MANGLE_SWITCH_AT        '*'
#define RULE_OP_MANGLE_CHR_SHIFTL       'L'
#define RULE_OP_MANGLE_CHR_SHIFTR       'R'
#define RULE_OP_MANGLE_CHR_INCR         '+'
#define RULE_OP_MANGLE_CHR_DECR         '-'
#define RULE_OP_MANGLE_REPLACE_NP1      '.'
#define RULE_OP_MANGLE_REPLACE_NM1      ','
#define RULE_OP_MANGLE_DUPEBLOCK_FIRST  'y'
#define RULE_OP_MANGLE_DUPEBLOCK_LAST   'Y'
#define RULE_OP_MANGLE_TITLE            'E'

#endif /* RULES_H */
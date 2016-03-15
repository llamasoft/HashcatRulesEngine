/**
 * Author......: Jens Steube <jens.steube@gmail.com>
 * License.....: MIT
 */

#include "rules.h"

// Increment rule position, throw error if premature end
#define NEXT_RULEPOS(rp)      do { if (++(rp) == rule_len) { return(RULE_RC_SYNTAX_ERROR); } } while (0)

// Read current rule position to integer value
#define NEXT_RPTOI(r,rp,up)   do { if ( ((up) = conv_ctoi( (r)[(rp)] )) == -1) { return(RULE_RC_SYNTAX_ERROR); } } while (0)

#define MANGLE_TOGGLE_AT(a,p) do { if ( class_alpha((a)[(p)]) ) { (a)[(p)] ^= 0x20; } } while (0)
#define MANGLE_LOWER_AT(a,p)  do { if ( class_upper((a)[(p)]) ) { (a)[(p)] ^= 0x20; } } while (0)
#define MANGLE_UPPER_AT(a,p)  do { if ( class_lower((a)[(p)]) ) { (a)[(p)] ^= 0x20; } } while (0)
#define MANGLE_SWITCH(a,l,r)  do { char c = (a)[(r)]; (a)[(r)] = (a)[(l)]; (a)[(l)] = c; } while (0)

bool class_num(char c)   { return((c >= '0') && (c <= '9')); }
bool class_lower(char c) { return((c >= 'a') && (c <= 'z')); }
bool class_upper(char c) { return((c >= 'A') && (c <= 'Z')); }
bool class_alpha(char c) { return(class_lower(c) || class_upper(c)); }

// Single character to integer value
// 0 .. 9 =>  0 ..  9
// A .. Z => 10 .. 35
// a .. z => 36 .. 61
// else -1 (error)
char conv_ctoi(char c)
{
    if (class_num(c)) {
        return c - '0';

    } else if (class_upper(c)) {
        return c - 'A' + (char) 10;

    } else if (class_lower(c)) {
        return c - 'a' + (char) 36;

    } else {
        return((char)(-1));
    }
}

// Convert to lower
int mangle_lrest(char arr[BLOCK_SIZE], int arr_len)
{
    int pos;
    for (pos = 0; pos < arr_len; pos++) { MANGLE_LOWER_AT(arr, pos); }

    return(arr_len);
}

// Convert to upper
int mangle_urest(char arr[BLOCK_SIZE], int arr_len)
{
    int pos;
    for (pos = 0; pos < arr_len; pos++) { MANGLE_UPPER_AT(arr, pos); }

    return(arr_len);
}

// Toggle case
int mangle_trest(char arr[BLOCK_SIZE], int arr_len)
{
    int pos;
    for (pos = 0; pos < arr_len; pos++) { MANGLE_TOGGLE_AT(arr, pos); }

    return(arr_len);
}

// Reverse a string
int mangle_reverse(char arr[BLOCK_SIZE], int arr_len)
{
    int a, z;
    for (a = 0; a < arr_len; a++) {
        z = arr_len - 1 - a;
        if (a >= z) { break; }

        MANGLE_SWITCH(arr, a, z);
    }

    return(arr_len);
}

// Append a string to itself
int mangle_double(char arr[BLOCK_SIZE], int arr_len)
{
    if ((arr_len * 2) >= BLOCK_SIZE) { return(arr_len); }

    memcpy(&arr[arr_len], arr, (size_t) arr_len);
    return(arr_len * 2);
}

// Append a string to itself N times
int mangle_double_times(char arr[BLOCK_SIZE], int arr_len, int times)
{
    if ((arr_len * times) + arr_len >= BLOCK_SIZE) { return(arr_len); }

    int orig_len = arr_len;

    int i;
    for (i = 0; i < times; i++) {
        memcpy(&arr[arr_len], arr, orig_len);

        arr_len += orig_len;
    }

    return(arr_len);
}

// Append a string to itself backwards
int mangle_reflect(char arr[BLOCK_SIZE], int arr_len)
{
    if ((arr_len * 2) >= BLOCK_SIZE) { return(arr_len); }

    mangle_double(arr, arr_len);
    mangle_reverse(arr + arr_len, arr_len);

    return(arr_len * 2);
}

// Rotates a string left one character
int mangle_rotate_left(char arr[BLOCK_SIZE], int arr_len)
{
    int a, z;
    for (a = 0, z = arr_len - 1; z > a; z--) {
        MANGLE_SWITCH(arr, a, z);
    }

    return(arr_len);
}

// Rotates a string right one character
int mangle_rotate_right(char arr[BLOCK_SIZE], int arr_len)
{
    int a, z;
    for (a = 0, z = arr_len - 1; a < z; a++) {
        MANGLE_SWITCH(arr, a, z);
    }

    return(arr_len);
}

// Appends a single character to a string
int mangle_append(char arr[BLOCK_SIZE], int arr_len, char c)
{
    if ((arr_len + 1) >= BLOCK_SIZE) { return(arr_len); }

    arr[arr_len] = c;

    return(arr_len + 1);
}

// Prepends a single character to a string
int mangle_prepend(char arr[BLOCK_SIZE], int arr_len, char c)
{
    if ((arr_len + 1) >= BLOCK_SIZE) { return(arr_len); }

    int arr_pos;
    for (arr_pos = arr_len - 1; arr_pos > -1; arr_pos--) {
        arr[arr_pos + 1] = arr[arr_pos];
    }

    arr[0] = c;

    return(arr_len + 1);
}

// Deletes a single character at upos
int mangle_delete_at(char arr[BLOCK_SIZE], int arr_len, int upos)
{
    if (upos >= arr_len) { return(arr_len); }

    int arr_pos;
    for (arr_pos = upos; arr_pos < arr_len - 1; arr_pos++) {
        arr[arr_pos] = arr[arr_pos + 1];
    }

    return(arr_len - 1);
}

// Replaces string with ulen characters starting at upos
int mangle_extract(char arr[BLOCK_SIZE], int arr_len, int upos, int ulen)
{
    if (upos >= arr_len) { return(arr_len); }
    if ((upos + ulen) > arr_len) { return(arr_len); }

    int arr_pos;
    for (arr_pos = 0; arr_pos < ulen; arr_pos++) {
        arr[arr_pos] = arr[upos + arr_pos];
    }

    return(ulen);
}

// Removes ulen characters starting at upos
int mangle_omit(char arr[BLOCK_SIZE], int arr_len, int upos, int ulen)
{
    if (upos >= arr_len) { return(arr_len); }
    if ((upos + ulen) > arr_len) { return(arr_len); }

    int arr_pos;
    for (arr_pos = upos; arr_pos < arr_len - ulen; arr_pos++) {
        arr[arr_pos] = arr[arr_pos + ulen];
    }

    return(arr_len - ulen);
}

// Inserts a single character at upos, shifting the result down
int mangle_insert(char arr[BLOCK_SIZE], int arr_len, int upos, char c)
{
    if (upos > arr_len) { return(arr_len); }
    if ((arr_len + 1) >= BLOCK_SIZE) { return(arr_len); }

    int arr_pos;
    for (arr_pos = arr_len - 1; arr_pos > upos - 1; arr_pos--) {
        arr[arr_pos + 1] = arr[arr_pos];
    }

    arr[upos] = c;

    return(arr_len + 1);
}

// Insert arr2_cpy characters from arr2 starting at position arr2_pos into position arr_pos
// arr[0 .. arr_pos - 1] + arr2[arr2_pos .. arr2_pos + arr2_cpy] + arr[arr_pos .. arr_len]
int mangle_insert_multi(char arr[BLOCK_SIZE], int arr_len, int arr_pos, char arr2[BLOCK_SIZE], int arr2_len, int arr2_pos, int arr2_cpy)
{
    if ((arr_len  + arr2_cpy) > BLOCK_SIZE) { return(RULE_RC_REJECT_ERROR); }
    if ((arr2_pos + arr2_cpy) > arr2_len  ) { return(RULE_RC_REJECT_ERROR); }

    if ( arr_pos >  arr_len) { return(RULE_RC_REJECT_ERROR); }
    if (arr2_pos > arr2_len) { return(RULE_RC_REJECT_ERROR); }

    if (arr2_cpy < 1) { return(RULE_RC_SYNTAX_ERROR); }

    // Shift arr2 down arr2_pos characters
    // This is the substring we will add to arr
    //   arr2[arr2_pos .. arr2_pos + arr2_cpy]
    memcpy(arr2, arr2 + arr2_pos, arr2_len - arr2_pos);

    // Append the back half of arr (after arr_pos) to arr2
    // This will become the back half of the result
    //   arr2[arr2_pos .. arr2_pos + arr2_cpy] + arr[arr_pos .. arr_len]
    memcpy(arr2 + arr2_cpy, arr + arr_pos, arr_len - arr_pos);

    // Insert our result to the correct place in arr
    memcpy(arr + arr_pos, arr2, arr_len - arr_pos + arr2_cpy);

    return(arr_len + arr2_cpy);
}

// Replace a single character at upos
int mangle_overstrike(char arr[BLOCK_SIZE], int arr_len, int upos, char c)
{
    if (upos >= arr_len) { return(arr_len); }

    arr[upos] = c;

    return(arr_len);
}

// Remove everything after position upos
int mangle_truncate_at(char arr[BLOCK_SIZE], int arr_len, int upos)
{
    if (upos >= arr_len) { return(arr_len); }

    memset(arr + upos, 0, arr_len - upos);

    return(upos);
}

// Replace all instances of oldc with newc
int mangle_replace(char arr[BLOCK_SIZE], int arr_len, char oldc, char newc)
{
    int arr_pos;
    for (arr_pos = 0; arr_pos < arr_len; arr_pos++) {
        if (arr[arr_pos] != oldc) { continue; }

        arr[arr_pos] = newc;
    }

    return(arr_len);
}

// Remove all instances of c
int mangle_purgechar(char arr[BLOCK_SIZE], int arr_len, char c)
{
    int arr_pos, ret_len;
    for (ret_len = 0, arr_pos = 0; arr_pos < arr_len; arr_pos++) {
        if (arr[arr_pos] == c) { continue; }

        arr[ret_len] = arr[arr_pos];

        ret_len++;
    }

    return(ret_len);
}

// Duplicate the first ulen characters, prepending them to the string
//   arr = "Apple", ulen = 3, result = "AppApple"
// TODO: optimize with memcpy()
int mangle_dupeblock_prepend(char arr[BLOCK_SIZE], int arr_len, int ulen)
{
    if (ulen > arr_len) { return(arr_len); }
    if ((arr_len + ulen) >= BLOCK_SIZE) { return(arr_len); }

    char cs[BLOCK_SIZE];
    memcpy(cs, arr, ulen);

    int i;
    for (i = 0; i < ulen; i++) {
        char c = cs[i];
        arr_len = mangle_insert(arr, arr_len, i, c);
    }

    return(arr_len);
}

// Duplicate the first ulen characters, appending them to the string
//   arr = "Apple", ulen = 3, result = "AppleApp"
// TODO: optimize with memcpy()
int mangle_dupeblock_append(char arr[BLOCK_SIZE], int arr_len, int ulen)
{
    if (ulen > arr_len) { return(arr_len); }
    if ((arr_len + ulen) >= BLOCK_SIZE) { return(arr_len); }

    int upos = arr_len - ulen;
    int i;
    for (i = 0; i < ulen; i++) {
        char c = arr[upos + i];

        arr_len = mangle_append(arr, arr_len, c);
    }

    return(arr_len);
}

// Duplicate the character at upos ulen times
int mangle_dupechar_at(char arr[BLOCK_SIZE], int arr_len, int upos, int ulen)
{
    if (arr_len == 0) { return(arr_len); }
    if ((arr_len + ulen) >= BLOCK_SIZE) { return(arr_len); }

    char c = arr[upos];
    int i;
    for (i = 0; i < ulen; i++) {
        arr_len = mangle_insert(arr, arr_len, upos, c);
    }

    return(arr_len);
}

// Duplicates every character
int mangle_dupechar(char arr[BLOCK_SIZE], int arr_len)
{
    if (arr_len == 0) { return(arr_len); }
    if ((arr_len + arr_len) >= BLOCK_SIZE) { return(arr_len); }

    int arr_pos;
    for (arr_pos = arr_len - 1; arr_pos > -1; arr_pos--) {
        int new_pos = arr_pos * 2;

        arr[new_pos] = arr[arr_pos];

        arr[new_pos + 1] = arr[arr_pos];
    }

    return(arr_len * 2);
}

// Swap the characters at positions upos and upos2
int mangle_switch_at_check(char arr[BLOCK_SIZE], int arr_len, int upos, int upos2)
{
    if (upos  >= arr_len) { return(arr_len); }
    if (upos2 >= arr_len) { return(arr_len); }

    MANGLE_SWITCH(arr, upos, upos2);

    return(arr_len);
}

// Swap the characters at positions upos and upos2, no safety checks
int mangle_switch_at(char arr[BLOCK_SIZE], int arr_len, int upos, int upos2)
{
    MANGLE_SWITCH(arr, upos, upos2);

    return(arr_len);
}

// Left bit-shift the character at upos
int mangle_chr_shiftl(uint8_t arr[BLOCK_SIZE], int arr_len, int upos)
{
    if (upos >= arr_len) { return(arr_len); }

    arr[upos] <<= 1;

    return(arr_len);
}

// Right bit-shift the character at upos
int mangle_chr_shiftr(uint8_t arr[BLOCK_SIZE], int arr_len, int upos)
{
    if (upos >= arr_len) { return(arr_len); }

    arr[upos] >>= 1;

    return(arr_len);
}

// Increment the character at upos
int mangle_chr_incr(uint8_t arr[BLOCK_SIZE], int arr_len, int upos)
{
    if (upos >= arr_len) { return(arr_len); }

    arr[upos] += 1;

    return(arr_len);
}

// Decrement the character at upos
int mangle_chr_decr(uint8_t arr[BLOCK_SIZE], int arr_len, int upos)
{
    if (upos >= arr_len) { return(arr_len); }

    arr[upos] -= 1;

    return(arr_len);
}

// Convert a string to title case
int mangle_title(char arr[BLOCK_SIZE], int arr_len)
{
    int upper_next = 1;

    int pos;
    for (pos = 0; pos < arr_len; pos++) {
        if (arr[pos] == ' ') {
            upper_next = 1;
            continue;
        }

        if (upper_next) {
            upper_next = 0;
            MANGLE_UPPER_AT(arr, pos);

        } else {
            MANGLE_LOWER_AT(arr, pos);
        }
    }

    return(arr_len);
}


int apply_rule(char *rule, int rule_len, char in[BLOCK_SIZE], int in_len, char out[BLOCK_SIZE])
{
    if (in  == NULL) { return(RULE_RC_REJECT_ERROR); }
    if (out == NULL) { return(RULE_RC_REJECT_ERROR); }
    if (in_len   < 1) { return(RULE_RC_REJECT_ERROR); }
    if (rule_len < 1) { return(RULE_RC_REJECT_ERROR); }

    char mem[BLOCK_SIZE];
    int out_len = in_len;
    int mem_len = 0;

    memcpy(out, in, out_len);

    int rule_pos;
    for (rule_pos = 0; rule_pos < rule_len; rule_pos++) {
        int upos, upos2, ulen;

        switch (rule[rule_pos]) {
            case ' ':
                break;

            case RULE_OP_MANGLE_NOOP:
                break;

            case RULE_OP_MANGLE_LREST:
                out_len = mangle_lrest(out, out_len);
                break;

            case RULE_OP_MANGLE_UREST:
                out_len = mangle_urest(out, out_len);
                break;

            case RULE_OP_MANGLE_LREST_UFIRST:
                out_len = mangle_lrest(out, out_len);

                if (out_len) { MANGLE_UPPER_AT(out, 0); }
                break;

            case RULE_OP_MANGLE_UREST_LFIRST:
                out_len = mangle_urest(out, out_len);

                if (out_len) { MANGLE_LOWER_AT(out, 0); }
                break;

            case RULE_OP_MANGLE_TREST:
                out_len = mangle_trest(out, out_len);
                break;

            case RULE_OP_MANGLE_TOGGLE_AT:
                NEXT_RULEPOS(rule_pos);
                NEXT_RPTOI(rule, rule_pos, upos);

                if (upos < out_len) { MANGLE_TOGGLE_AT(out, upos); }
                break;

            case RULE_OP_MANGLE_REVERSE:
                out_len = mangle_reverse(out, out_len);
                break;

            case RULE_OP_MANGLE_DUPEWORD:
                out_len = mangle_double(out, out_len);
                break;

            case RULE_OP_MANGLE_DUPEWORD_TIMES:
                NEXT_RULEPOS(rule_pos);
                NEXT_RPTOI(rule, rule_pos, ulen);

                out_len = mangle_double_times(out, out_len, ulen);
                break;

            case RULE_OP_MANGLE_REFLECT:
                out_len = mangle_reflect(out, out_len);
                break;

            case RULE_OP_MANGLE_ROTATE_LEFT:
                mangle_rotate_left(out, out_len);
                break;

            case RULE_OP_MANGLE_ROTATE_RIGHT:
                mangle_rotate_right(out, out_len);
                break;

            case RULE_OP_MANGLE_APPEND:
                NEXT_RULEPOS(rule_pos);

                out_len = mangle_append(out, out_len, rule[rule_pos]);
                break;

            case RULE_OP_MANGLE_PREPEND:
                NEXT_RULEPOS(rule_pos);

                out_len = mangle_prepend(out, out_len, rule[rule_pos]);
                break;

            case RULE_OP_MANGLE_DELETE_FIRST:
                out_len = mangle_delete_at(out, out_len, 0);
                break;

            case RULE_OP_MANGLE_DELETE_LAST:
                out_len = mangle_delete_at(out, out_len, (out_len) ? out_len - 1 : 0);
                break;

            case RULE_OP_MANGLE_DELETE_AT:
                NEXT_RULEPOS(rule_pos);
                NEXT_RPTOI(rule, rule_pos, upos);

                out_len = mangle_delete_at(out, out_len, upos);
                break;

            case RULE_OP_MANGLE_EXTRACT:
                NEXT_RULEPOS(rule_pos);
                NEXT_RPTOI(rule, rule_pos, upos);

                NEXT_RULEPOS(rule_pos);
                NEXT_RPTOI(rule, rule_pos, ulen);

                out_len = mangle_extract(out, out_len, upos, ulen);
                break;

            case RULE_OP_MANGLE_OMIT:
                NEXT_RULEPOS(rule_pos);
                NEXT_RPTOI(rule, rule_pos, upos);

                NEXT_RULEPOS(rule_pos);
                NEXT_RPTOI(rule, rule_pos, ulen);

                out_len = mangle_omit(out, out_len, upos, ulen);
                break;

            case RULE_OP_MANGLE_INSERT:
                NEXT_RULEPOS(rule_pos);
                NEXT_RPTOI(rule, rule_pos, upos);

                NEXT_RULEPOS(rule_pos);

                out_len = mangle_insert(out, out_len, upos, rule[rule_pos]);
                break;

            case RULE_OP_MANGLE_OVERSTRIKE:
                NEXT_RULEPOS(rule_pos);
                NEXT_RPTOI(rule, rule_pos, upos);

                NEXT_RULEPOS(rule_pos);

                out_len = mangle_overstrike(out, out_len, upos, rule[rule_pos]);
                break;

            case RULE_OP_MANGLE_TRUNCATE_AT:
                NEXT_RULEPOS(rule_pos);
                NEXT_RPTOI(rule, rule_pos, upos);

                out_len = mangle_truncate_at(out, out_len, upos);
                break;

            case RULE_OP_MANGLE_REPLACE:
                NEXT_RULEPOS(rule_pos);
                NEXT_RULEPOS(rule_pos);

                out_len = mangle_replace(out, out_len, rule[rule_pos - 1], rule[rule_pos]);
                break;

            case RULE_OP_MANGLE_PURGECHAR:
                NEXT_RULEPOS(rule_pos);

                out_len = mangle_purgechar(out, out_len, rule[rule_pos]);
                break;

            case RULE_OP_MANGLE_DUPECHAR_FIRST:
                NEXT_RULEPOS(rule_pos);
                NEXT_RPTOI(rule, rule_pos, ulen);

                out_len = mangle_dupechar_at(out, out_len, 0, ulen);
                break;

            case RULE_OP_MANGLE_DUPECHAR_LAST:
                NEXT_RULEPOS(rule_pos);
                NEXT_RPTOI(rule, rule_pos, ulen);

                out_len = mangle_dupechar_at(out, out_len, out_len - 1, ulen);
                break;

            case RULE_OP_MANGLE_DUPECHAR_ALL:
                out_len = mangle_dupechar(out, out_len);
                break;

            case RULE_OP_MANGLE_DUPEBLOCK_FIRST:
                NEXT_RULEPOS(rule_pos);
                NEXT_RPTOI(rule, rule_pos, ulen);

                out_len = mangle_dupeblock_prepend(out, out_len, ulen);
                break;

            case RULE_OP_MANGLE_DUPEBLOCK_LAST:
                NEXT_RULEPOS(rule_pos);
                NEXT_RPTOI(rule, rule_pos, ulen);

                out_len = mangle_dupeblock_append(out, out_len, ulen);
                break;

            case RULE_OP_MANGLE_SWITCH_FIRST:
                if (out_len >= 2) { mangle_switch_at(out, out_len, 0, 1); }
                break;

            case RULE_OP_MANGLE_SWITCH_LAST:
                if (out_len >= 2) { mangle_switch_at(out, out_len, out_len - 1, out_len - 2); }
                break;

            case RULE_OP_MANGLE_SWITCH_AT:
                NEXT_RULEPOS(rule_pos);
                NEXT_RPTOI(rule, rule_pos, upos);

                NEXT_RULEPOS(rule_pos);
                NEXT_RPTOI(rule, rule_pos, upos2);

                out_len = mangle_switch_at_check(out, out_len, upos, upos2);
                break;

            case RULE_OP_MANGLE_CHR_SHIFTL:
                NEXT_RULEPOS(rule_pos);
                NEXT_RPTOI(rule, rule_pos, upos);

                mangle_chr_shiftl((uint8_t *) out, out_len, upos);
                break;

            case RULE_OP_MANGLE_CHR_SHIFTR:
                NEXT_RULEPOS(rule_pos);
                NEXT_RPTOI(rule, rule_pos, upos);

                mangle_chr_shiftr((uint8_t *) out, out_len, upos);
                break;

            case RULE_OP_MANGLE_CHR_INCR:
                NEXT_RULEPOS(rule_pos);
                NEXT_RPTOI(rule, rule_pos, upos);

                mangle_chr_incr((uint8_t *) out, out_len, upos);
                break;

            case RULE_OP_MANGLE_CHR_DECR:
                NEXT_RULEPOS(rule_pos);
                NEXT_RPTOI(rule, rule_pos, upos);

                mangle_chr_decr((uint8_t *) out, out_len, upos);
                break;

            case RULE_OP_MANGLE_REPLACE_NP1:
                NEXT_RULEPOS(rule_pos);
                NEXT_RPTOI(rule, rule_pos, upos);

                if ((upos >= 0) && ((upos + 1) < out_len)) { mangle_overstrike(out, out_len, upos, out[upos + 1]); }
                break;

            case RULE_OP_MANGLE_REPLACE_NM1:
                NEXT_RULEPOS(rule_pos);
                NEXT_RPTOI(rule, rule_pos, upos);

                if ((upos >= 1) && ((upos + 0) < out_len)) { mangle_overstrike(out, out_len, upos, out[upos - 1]); }
                break;

            case RULE_OP_MANGLE_TITLE:
                out_len = mangle_title(out, out_len);
                break;

            case RULE_OP_MANGLE_EXTRACT_MEMORY:
                if (mem_len < 1) { return(RULE_RC_REJECT_ERROR); }

                NEXT_RULEPOS(rule_pos);
                NEXT_RPTOI(rule, rule_pos, upos);

                NEXT_RULEPOS(rule_pos);
                NEXT_RPTOI(rule, rule_pos, ulen);

                NEXT_RULEPOS(rule_pos);
                NEXT_RPTOI(rule, rule_pos, upos2);

                if ((out_len = mangle_insert_multi(out, out_len, upos2, mem, mem_len, upos, ulen)) < 1) { return(out_len); }
                break;

            case RULE_OP_MANGLE_APPEND_MEMORY:
                if (mem_len < 1) { return(RULE_RC_REJECT_ERROR); }
                if ((out_len + mem_len) > BLOCK_SIZE) { return(RULE_RC_REJECT_ERROR); }

                memcpy(out + out_len, mem, mem_len);
                out_len += mem_len;
                break;

            case RULE_OP_MANGLE_PREPEND_MEMORY:
                if (mem_len < 1) { return(RULE_RC_REJECT_ERROR); }
                if ((mem_len + out_len) > BLOCK_SIZE) { return(RULE_RC_REJECT_ERROR); }

                // Use the unused portion of mem as a buffer area
                // We know it fits because (mem_len + out_len <= BLOCK_SIZE)
                memcpy(mem + mem_len, out, out_len);
                out_len += mem_len;
                memcpy(out, mem, out_len);
                break;

            case RULE_OP_MEMORIZE_WORD:
                memcpy(mem, out, out_len);
                mem_len = out_len;
                break;

            case RULE_OP_REJECT_LESS:
                NEXT_RULEPOS(rule_pos);
                NEXT_RPTOI(rule, rule_pos, upos);

                if (out_len > upos) { return(RULE_RC_REJECT_ERROR); }
                break;

            case RULE_OP_REJECT_GREATER:
                NEXT_RULEPOS(rule_pos);
                NEXT_RPTOI(rule, rule_pos, upos);

                if (out_len < upos) { return(RULE_RC_REJECT_ERROR); }
                break;

            case RULE_OP_REJECT_CONTAIN:
                NEXT_RULEPOS(rule_pos);

                if (strchr(out, rule[rule_pos]) != NULL) { return(RULE_RC_REJECT_ERROR); }
                break;

            case RULE_OP_REJECT_NOT_CONTAIN:
                NEXT_RULEPOS(rule_pos);

                if (strchr(out, rule[rule_pos]) == NULL) { return(RULE_RC_REJECT_ERROR); }
                break;

            case RULE_OP_REJECT_EQUAL_FIRST:
                NEXT_RULEPOS(rule_pos);

                if (out[0] != rule[rule_pos]) { return(RULE_RC_REJECT_ERROR); }
                break;

            case RULE_OP_REJECT_EQUAL_LAST:
                NEXT_RULEPOS(rule_pos);

                if (out[out_len - 1] != rule[rule_pos]) { return(RULE_RC_REJECT_ERROR); }
                break;

            case RULE_OP_REJECT_EQUAL_AT:
                NEXT_RULEPOS(rule_pos);
                NEXT_RPTOI(rule, rule_pos, upos);
                if ((upos + 1) > out_len) { return(RULE_RC_REJECT_ERROR); }

                NEXT_RULEPOS(rule_pos);
                if (out[upos] != rule[rule_pos]) { return(RULE_RC_REJECT_ERROR); }

                break;

            case RULE_OP_REJECT_CONTAINS:
                NEXT_RULEPOS(rule_pos);
                NEXT_RPTOI(rule, rule_pos, upos);
                if ((upos + 1) > out_len) { return(RULE_RC_REJECT_ERROR); }

                NEXT_RULEPOS(rule_pos);
                int c, cnt; for (c = 0, cnt = 0; c < out_len; c++) { if (out[c] == rule[rule_pos]) { cnt++; } }
                if (cnt < upos) { return(RULE_RC_REJECT_ERROR); }
                break;

            case RULE_OP_REJECT_MEMORY:
                if (out_len == mem_len && memcmp(out, mem, out_len) == 0) { return(RULE_RC_REJECT_ERROR); }
                break;

            default:
                return(RULE_RC_SYNTAX_ERROR);
                break;
        }
    }

    memset(out + out_len, 0, BLOCK_SIZE - out_len);
    return(out_len);
}
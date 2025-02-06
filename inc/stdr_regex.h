#ifndef REGEX_H_
#define REGEX_H_

#include "stdr.h"

typedef u64 regex_state_t;

enum regex_match {
  MATCH_LA,
  MATCH_LB,
  MATCH_LC,
  MATCH_LM,
  MATCH_LU,
  MATCH_LL,
  MATCH_LZ,
  MATCH_UZ,
  MATCH_COMMA,
  MATCH_WILDCARD,
  MATCH_ASTERIX,
  MATCH_PLUS,
  MATCH_PAREN_OPEN,
  MATCH_PAREN_CLOSE,
  MATCH_SKIP,
  MATCH_BSLASH,
  MATCH_DIGIT,
  MATCH_COUNT,
};

typedef struct {
  regex_state_t from;
  regex_state_t to;
  enum regex_match match;
} regex_transition_t;

typedef struct {
  arr(regex_state_t) states;
  arr(regex_transition_t) transitions;
  arr(regex_state_t) env;
} regex_t;

void regex_delete(regex_t* regex);
void regex_generate_dot(regex_t* regex, const cstr_t filename);
void regex_compile(regex_t* regex, regex_state_t s0, str_t rs);
regex_state_t regex_push_state(regex_t* regex);
void regex_push_transition(regex_t* regex, regex_state_t from, regex_state_t to,
                           enum regex_match match);
bool regex_match_ch(enum regex_match match, char ch);
enum regex_match regex_match_parse(char ch);
str_t regex_match_str_start_impl(const regex_t* regex, regex_state_t begin,
                                 str_t str, usize count, char* sstr);
str_t regex_match_str_start(regex_t* regex, regex_state_t s0, str_t str);
str_t regex_match_str(regex_t* regex, regex_state_t s0, str_t str,
                      str_t* out_rest);

#endif

#ifdef STDR_REGEX_IMPLEMENTATION

const str_t REGEX_MATCH_LITERALS[] = {
    [MATCH_LA] = str("a"),         [MATCH_LB] = str("b"),
    [MATCH_LC] = str("c"),         [MATCH_LM] = str("m"),
    [MATCH_LU] = str("u"),         [MATCH_LL] = str("l"),
    [MATCH_LZ] = str("z"),         [MATCH_UZ] = str("Z"),
    [MATCH_COMMA] = str(","),      [MATCH_WILDCARD] = str("."),
    [MATCH_ASTERIX] = str("*"),    [MATCH_PLUS] = str("+"),
    [MATCH_PAREN_OPEN] = str("("), [MATCH_PAREN_CLOSE] = str(")"),
    [MATCH_SKIP] = str("SKIP"),    [MATCH_BSLASH] = str("\\"),
    [MATCH_DIGIT] = str("DIGIT")};

regex_state_t regex_push_state(regex_t* regex) {
  regex_state_t s0 = (regex_state_t)arr_count(regex->states);
  arr_append(regex->states, s0);
  return s0;
}

void regex_push_transition(regex_t* regex, regex_state_t from, regex_state_t to,
                           enum regex_match match) {
  arr_append(regex->transitions,
             (regex_transition_t){.from = from, .to = to, .match = match});
}

bool regex_match_ch(enum regex_match match, char ch) {
  if (match == MATCH_WILDCARD) return true;
  if (match == MATCH_DIGIT) return (ch >= '0' && ch <= '9');
  if (REGEX_MATCH_LITERALS[match].len != 1) assert(0);
  return REGEX_MATCH_LITERALS[match].ptr[0] == ch;

  assert(0 && "unreachable");
}

str_t regex_match_str(regex_t* regex, regex_state_t s0, str_t str,
                      str_t* out_rest) {
  while (true) {
    if (str.len == 0) return STR_NULL;

    str_t match = regex_match_str_start(regex, s0, str);

    if (str_is_null(match)) {
      str = str_drop(str, 1);
      *out_rest = str;
      continue;
    } else {
      *out_rest = str_drop(str, match.len);
      return match;
    }
  }

  return STR_NULL;
}

str_t regex_match_str_start(regex_t* regex, regex_state_t s0, str_t str) {
  return regex_match_str_start_impl(regex, s0, str, 0, str.ptr);
}

str_t regex_match_str_start_impl(const regex_t* regex, regex_state_t begin,
                                 str_t str, usize count, char* sstr) {
  STDR_ASSERT(arr_count(regex->states) >= 2);

  if (begin == arr_last(regex->states))
    return (str_t){.len = count, .ptr = sstr};

  for (usize i = 0; i < arr_count(regex->transitions); i++) {
    regex_transition_t trans = regex->transitions[i];
    if (trans.from != begin) continue;
    if (trans.match == MATCH_SKIP) {
      str_t d =
          regex_match_str_start_impl(regex, trans.to, str, count + 1, sstr);
      if (d.ptr != NULL) return d;
    }

    if (!regex_match_ch(trans.match, str.ptr[0])) continue;

    str_t d = regex_match_str_start_impl(regex, trans.to, str_drop(str, 1),
                                         count + 1, sstr);
    if (d.ptr != NULL) return d;
  }

  return STR_NULL;
}

void regex_delete(regex_t* regex) {
  arr_free(regex->states);
  arr_free(regex->transitions);
  arr_free(regex->env);
}

void regex_generate_dot(regex_t* regex, const cstr_t filename) {
  FILE* f = fopen(filename, "w+");
  assert(f != NULL);
  fprintf(f, "digraph regex {\n");
  fprintf(f, "  rankdir=\"LR\";\n");
  for (usize i = 0; i < arr_count(regex->transitions); i++) {
    regex_transition_t trans = regex->transitions[i];

    fprintf(f, "s%ld -> s%ld [ label=\"%s\"]\n", trans.from, trans.to,
            REGEX_MATCH_LITERALS[trans.match].ptr);
  }
  fprintf(f, "}\n");
  fclose(f);

  cmd("dot", "-Tsvg", filename, "-o", "regex.svg");
}

enum regex_match regex_match_parse(char ch) {
  for (usize i = 0; i < MATCH_COUNT; i++) {
    if (REGEX_MATCH_LITERALS[i].len != 1) continue;

    if (ch == REGEX_MATCH_LITERALS[i].ptr[0]) {
      return (enum regex_match)i;
    }
  }

  printf("[ERROR] Unknown character '%c' (%d)\n", ch, ch);
  assert(0);
}

void regex_compile(regex_t* regex, regex_state_t s0, str_t rs) {
  if (rs.len == 0) return;

  enum regex_match match = regex_match_parse(rs.ptr[0]);
  switch (match) {
    case MATCH_BSLASH: {
      if (rs.ptr[1] == 'd') {
        regex_state_t n = regex_push_state(regex);
        regex_push_transition(regex, s0, n, MATCH_DIGIT);
        regex_compile(regex, n, str_drop(rs, 2));
      } else {
        assert(0);
      }
    } break;
    case MATCH_PLUS: {
      regex_transition_t t = arr_last(regex->transitions);
      regex_push_transition(regex, s0, s0, t.match);
      regex_compile(regex, s0, str_drop(rs, 1));
    } break;
    case MATCH_ASTERIX: {
      regex_state_t s1 = arr_pre_last(regex->states);
      regex_state_t n = regex_push_state(regex);
      regex_push_transition(regex, s1, n, MATCH_SKIP);

      regex_transition_t t0 = arr_pre_last(regex->transitions);
      regex_push_transition(regex, s0, s0, t0.match);
      regex_push_transition(regex, s0, n, MATCH_SKIP);
      regex_compile(regex, n, str_drop(rs, 1));
    } break;
    default: {
      regex_state_t n = regex_push_state(regex);
      regex_push_transition(regex, s0, n, match);
      regex_compile(regex, n, str_drop(rs, 1));
    } break;
  }
}

#endif  // STDR_REGEX_IMPLEMENTATION
#undef STDR_REGEX_IMPLEMENTATION

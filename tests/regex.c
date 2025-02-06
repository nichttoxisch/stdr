#include <stdio.h>

#define STDR_IMPLEMENTATION
#include "stdr.h"
#define STDR_REGEX_IMPLEMENTATION
#include "stdr_regex.h"

int main(void) {
  regex_t regex = {0};

  // Begin State
  regex_state_t s0 = regex_push_state(&regex);
  regex_compile(
      &regex, s0,
      str("mul(\\d+,\\d+)"));  // Note: parenthesis are character literals

  str_t input =
      str("notmul(112,2)+some mjunkmul(12,22)+some junkmul(61,2222)a");

  // Generate a dot graph and compile it
  regex_generate_dot(&regex, "data/regex.dot");

  while (true) {
    // Will look for thee first match in the string
    // The rest of the string is put back in input for reuse
    str_t match = regex_match_str(&regex, s0, input, &input);

    // No match is found
    if (str_is_null(match)) break;

    printf("%.*s\n", sfmt(match));
  }

  // Cleanup
  regex_delete(&regex);

  return 0;
}

#include <stdio.h>

#define STDR_FLAG_IMPLEMENTATION
#include "stdr_flag.h"

#define STDR_IMPLEMENTATION
#include "stdr.h"

i32 main(i32 argc, const cstr_t argv[]) {
  // There are three supported command line arguments (boolean string integer).

  // This is an example for a boolean argument:
  // Default value for argumnt must always be provided
  // Usage: -flag_name to set the target variable to true
  // TODO: -flag_name=true is not supported
  bool enable_feature = false;
  // To register a new command line flag
  stdr_flag_bool(&enable_feature, str("enable_feature"), str("Enable feature"));
  // This is just a shorthand. The same dst pointer can be used
  stdr_flag_bool(&enable_feature, str("ef"), str("Enable feature (shorthand)"));

  // Example for a string argument:
  // Usage: -flag_name "String input"
  //        -flag_name Gibberish
  str_t string_argument = STR_NULL;
  stdr_flag_str(&string_argument, str("reverse"),
                str("Prints the reversed string to stdout"));
  stdr_flag_str(&string_argument, str("r"),
                str("Prints the reversed string to stdout (shorthand)"));

  i64 counter = 3;
  stdr_flag_i64(&counter, str("count"),
                str("Will count from one up to provided argument"));
  stdr_flag_i64(&counter, str("c"),
                str("Will count from one up to provided argument (shorthand)"));

  // A help command (-help -h) is always registered and will list the flags,
  // descriptions and default values

  // usage: ./main [options]
  //       -enable_feature
  //               Enable feature
  //       -ef
  //               Enable feature (shorthand)
  //       -reverse <string>
  //               Prints the reversed string to stdout (default NULL)
  //       -r <string>
  //               Prints the reversed string to stdout (shorthand) (default
  //               NULL)
  //       -count <number>
  //               Will count from one up to provided argument (default 3)
  //       -c <number>
  //               Will count from one up to provided argument (shorthand)
  //               (default 3)

  // Before calling stdr_flag_parse() all flags must be registered
  stdr_flag_parse(argc, argv);

  // Here all prviously defined flags are valid.

  printf("Feature is %s\n", enable_feature ? "enabled" : "disabled");

  // If -r flag is provided print string reversed
  if (!str_is_null(string_argument)) {
    printf("r = %.*s\n", SFMT(string_argument));
    for (isize i = string_argument.len - 1; i >= 0; i -= 1) {
      printf("%c", string_argument.ptr[i]);
    }
    printf("\n");
  }

  // Count up to provided arguement (default 3)
  printf("counter = %lld\n", counter);
  for (isize i = 1; i <= counter; i++) {
    printf("%ld\n", i);
  }
  return 0;
}

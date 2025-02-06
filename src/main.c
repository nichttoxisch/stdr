#include <stdarg.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

#define STDR_IMPLEMENTATION
#include "stdr.h"

i32 main(i32 argc, const cstr_t argv[]) {
  (void)argc;
  (void)argv;

  cmd("ls", "-la");

  return 0;
}

#ifndef STDR_FLAG_H_
#define STDR_FLAG_H_

#include <assert.h>
#include <stdio.h>

#include "stdr.h"

void stdr_flag_bool(bool* dst, str_t name, str_t desciption);
void stdr_flag_str(str_t* dst, str_t name, str_t desciption);
void stdr_flag_i64(i64* dst, str_t name, str_t desciption);

void stdr_flag_parse(i32 argc, const cstr_t argv[]);

#endif  // STDR_FLAG_H_

#define STDR_FLAG_IMPLEMENTATION
#ifdef STDR_FLAG_IMPLEMENTATION

typedef enum {
  FK_UNDEFINED,
  FK_BOOL,
  FK_STR,
  FK_I64,
} stdr_flag_kind_t;

static cstr_t stdr_flag_king_str[] = {
    [FK_UNDEFINED] = "FK_UNDEFINED",
    [FK_BOOL] = "boolean",
    [FK_STR] = "string",
    [FK_I64] = "integer",
};

typedef struct {
  stdr_flag_kind_t kind;
  str_t name;
  str_t description;
  union {
    bool _bool;
    str_t _str;
    i64 _i64;
  } default_as;
  union {
    bool* _bool;
    str_t* _str;
    i64* _i64;
  } as;
} stdr_flag_t;

typedef struct {
  arr(stdr_flag_t) flags;
} stdr_flag_ctx_t;

static stdr_flag_ctx_t ctx;

void stdr_flag_bool(bool* dst, str_t name, str_t desciption) {
  stdr_flag_t f = {.kind = FK_BOOL,
                   .name = name,
                   .description = desciption,
                   .as._bool = dst,
                   .default_as._bool = *dst};

  arr_append(ctx.flags, f);
}

void stdr_flag_str(str_t* dst, str_t name, str_t desciption) {
  stdr_flag_t f = {.kind = FK_STR,
                   .name = name,
                   .description = desciption,
                   .as._str = dst,
                   .default_as._str = *dst};
  arr_append(ctx.flags, f);
}

void stdr_flag_i64(i64* dst, str_t name, str_t desciption) {
  stdr_flag_t f = {.kind = FK_I64,
                   .name = name,
                   .description = desciption,
                   .as._i64 = dst,
                   .default_as._i64 = *dst};

  arr_append(ctx.flags, f);
}

void stdr_flag_print(const cstr_t program, FILE* file) {
  fprintf(file, "usage: %s [options]\n", program);
  for (isize i = 0; i < arr_count(ctx.flags); i++) {
    stdr_flag_t f = ctx.flags[i];
    switch (f.kind) {
      case FK_BOOL: {
        fprintf(file, "\t-%.*s\n", SFMT(f.name));
        fprintf(file, "\t\t%.*s\n", SFMT(f.description));
      } break;
      case FK_I64: {
        fprintf(file, "\t-%.*s <number>\n", SFMT(f.name));
        fprintf(file, "\t\t%.*s ", SFMT(f.description));
        fprintf(file, "(default %lld)\n", f.default_as._i64);
      } break;
      case FK_STR: {
        fprintf(file, "\t-%.*s <string>\n", SFMT(f.name));
        fprintf(file, "\t\t%.*s ", SFMT(f.description));
        if (str_is_null(f.default_as._str)) {
          fprintf(file, "(default NULL)\n");
        } else {
          fprintf(file, "(default '%.*s')\n", SFMT(f.default_as._str));
        }
      } break;
      case FK_UNDEFINED: {
        assert(0 && "unreachable");
      } break;
    }
  }
}

void stdr_flag_parse(i32 argc, const cstr_t argv[]) {
  for (i32 i = 0; i < argc; i++) {
    str_t arg = str(argv[i]);

    if (str_eq(arg, str("-help")) || str_eq(arg, str("-h"))) {
      stdr_flag_print(argv[0], stderr);
      exit(0);
    }

    // TODO: Check for arguments that are not registered and throw an error

    if (arg.ptr[0] != '-') continue;
    arg = str_drop(arg, 1);

    for (isize j = 0; j < arr_count(ctx.flags); j++) {
      stdr_flag_t f = ctx.flags[j];

      if (str_eq(arg, f.name)) {
        switch (f.kind) {
          case FK_UNDEFINED: {
            assert(0 && "unreachable");
          } break;
          case FK_BOOL: {
            *f.as._bool = true;
          } break;
          case FK_STR: {
            if (i >= argc - 1) {
              fprintf(stderr,
                      "[ERROR] Parsing flag -%.*s: No argument provided! "
                      "Expected %s.\n",
                      SFMT(f.name), stdr_flag_king_str[f.kind]);
              exit(1);
            }
            i++;
            str_t s = str(argv[i]);

            *f.as._str = s;
          } break;
          case FK_I64: {
            if (i >= argc - 1) {
              fprintf(stderr,
                      "[ERROR] Parsing flag -%.*s: No argument provided! "
                      "Expected %s.\n",
                      SFMT(f.name), stdr_flag_king_str[f.kind]);
              exit(1);
            }
            i++;

            // TODO: Check if argument is a number
            str_t s = str(argv[i]);
            i64 n = str_parse_i64(s);
            *f.as._i64 = n;
          } break;
        }
      }
    }
  }
}

#endif  // STDR_FLAG_IMPLEMENTATION
#include <stdio.h>
#include <stdlib.h>

#define STDR_IMPLEMENTATION
#include "stdr.h"

typedef struct {
  str_t key;
  i64 value;
} pair_t;

int pair_cmp(const void *a, const void *b) {
  pair_t _a = *(pair_t *)a;
  pair_t _b = *(pair_t *)b;
  return (int)(_b.value - _a.value);
}

void print_first_n(arr(pair_t) acc, usize n) {
  printf("[");
  for (usize i = 0; i < n; i++) {
    if (i > 0) printf(", ");

    printf("('%.*s', %ld)", (int)acc[i].key.len, (char *)acc[i].key.ptr,
           acc[i].value);
  }
  printf("]\n");
}

void str_free(str_t s) { free(s.ptr); }

int main(i32 argc, const cstr_t argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
    exit(1);
  }

  dstr_t content = read_file("data/pride_and_prejudice.txt");
  arr(str_t) words = str_split_words(str(content));

  map(i64) dic = NULL;
  for (usize i = 0; i < arr_count(words); i++) {
    str_to_lowercase(words[i]);

    if (map_has(dic, words[i])) {
      *map_get(dic, words[i]) += 1;
    } else {
      map_insert(dic, words[i], 1);
    }
  }

  arr(pair_t) acc = NULL;
  map_items_collect(dic, acc);
  arr_sort(acc, pair_cmp);

  print_first_n(acc, 10);

  arr_free(acc);
  map_free(dic);
  arr_free(words);
  dstr_free(content);
}

# Header-only standard library for c

## Features

### Dynamic arrays and Hash maps

```c
#include <stdio.h>
#include <stdlib.h>

// Define STDR_IMPLEMENTATION in only one translation unit 
// Single file libraries: https://github.com/nothings/stb
// Several stdlib function can be set using a definition of
// STDR_ASSERT and STDR_MALLOC, STDR_FREE before this statement
#define STDR_IMPLEMENTATION
#include "stdr.h"

// Used by map_items_collect
typedef struct {
  // Always of type str_t. Expects the name key
  str_t key;
  // Expects same type as map that yyou try to collect
  i64 value;
} pair_t;

// Used by arr_sort. Standard comapring function passed to qsort from stdlib
int pair_cmp(const void *a, const void *b) {
  pair_t _a = *(pair_t *)a;
  pair_t _b = *(pair_t *)b;
  return (int)(_b.value - _a.value);
}

// Mimics the standart python3 output for easy comparison
void print_first_n(arr(pair_t) acc, usize n) {
  printf("[");
  for (usize i = 0; i < n; i++) {
    if (i > 0) printf(", ");

    printf("('%.*s', %ld)", (int)acc[i].key.len, (char *)acc[i].key.ptr,
           acc[i].value);
  }
  printf("]\n");
}

int main(i32 argc, const cstr_t argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
    exit(1);
  }

  const cstr_t filename = argv[1];

  // Reads the entire file into a dstr_t (arr(char)). Later must be freed with dstr_free
  dstr_t content = read_file(filename);
  
  // Splits a string at any whitespace
  // str() macro converts any char* will a null terminator into str_t
  // this could be arr(char) cstr_t or dstr_t
  // Initializes and allocates an array. Must be freed using arr_free
  arr(str_t) words = str_split_words(str(content));
  
  // Define a map with key of type str_t and value of type i64
  map(i64) dic = NULL;

  // Arrays provide access to a header with count and capacity
  // Can be read using arr_count and arr_capacity
  for (usize i = 0; i < arr_count(words); i++) {\
    // The stdr library provides several convenient funcitons
    str_to_lowercase(words[i]);

    // Count the accurance of each word
    if (map_has(dic, words[i])) {
      // Returns a pointer to the data. Here: i64*
      *map_get(dic, words[i]) += 1;
    } else {
      map_insert(dic, words[i], 1);
    }
  }

  arr(pair_t) acc = NULL;
  // Collects all the entries of the map into a paired array
  // Keey is thee entry and value is the count
  map_items_collect(dic, acc);

  // Sort the array in descending count to get the most common words
  arr_sort(acc, pair_cmp);

  print_first_n(acc, 10);
  
  // Free up allocated memory. fsanitizer works with source code
  arr_free(acc);
  map_free(dic);
  arr_free(words);
  dstr_free(content);

  return 0;
}

```

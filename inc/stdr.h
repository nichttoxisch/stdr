
#ifndef STDR_H_
#define STDR_H_ 1

#include <stdbool.h>
#include <stdint.h>
#include <string.h>     // memcpy, memcmp, strlen
#include <sys/types.h>  // ssize_t

#ifndef STDR_MALLOC

#ifdef STDR_FREE
#error "STDR_FREE must be defined with STDR_MALLOC"
#endif

#include <stdlib.h>  // malloc, free, qsort
#define STDR_MALLOC malloc
#define STDR_FREE free

#else  // STDR_MALLOC

#ifndef STDR_FREE
#error "STDR_FREE must be defined with STDR_MALLOC"
#endif  // STDR_FREE

#endif  // STDR_MALLOC

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef size_t usize;

typedef ssize_t isize;

typedef char *cstr_t;

bool is_space(char ch);
bool not_is_space(char ch);

typedef struct {
  char *ptr;
  usize len;
} str_t;

#define STR_NULL ((str_t){NULL, 0})

#define str_is_null(s) (s.ptr == NULL)

#define str(s) ((str_t){s, strlen(s)})
#define sfmt(s) (int)s.len, s.ptr

str_t str_alloc(usize len);
void str_free(str_t s);

str_t str_cpy(str_t s, void *dst);
str_t str_cpy_alloc(str_t s);

str_t str_drop(str_t str, usize n);
bool str_drop_while(str_t *str, bool (*f)(char));
bool str_split_at(str_t s, str_t *lhs, str_t *rhs, usize i);
bool str_split_while(str_t s, str_t *lhs, str_t *rhs, bool (*f)(char));

str_t str_trim_start(str_t s);

void str_to_lowercase(str_t s);
void str_to_uppercase(str_t s);

#ifndef STDR_HASH
usize stdr_hash(str_t k);
#define STDR_HASH stdr_hash
#endif

typedef struct {
  usize item_size;
  usize count;
  usize capacity;
} arr_header_t;

#define arr(T) T *

#define arr_header(a) (a == NULL ? NULL : ((arr_header_t *)a - 1))
#define arr_count(a) (a == NULL ? 0 : arr_header(a)->count)
#define arr_item_size(a) (a == NULL ? 0 : arr_header(a)->item_size)
#define arr_capacity(a) (a == NULL ? 0 : arr_header(a)->capacity)
#define arr_size(a) (arr_capacity(a) * arr_item_size(a))

#define arr_last(a) ((a)[arr_count(a) - 1])
#define arr_pre_last(a) ((a)[arr_count(a) - 2])

void arr_free(arr(void) a);
arr(void) arr_alloc(usize item_size, usize capacity);
arr(void) arr_realloc(arr(void) a, usize new_capacity);
usize capacity_grow(usize capacity);

#define arr_append(a, ...)                                             \
  do {                                                                 \
    if ((a) == NULL) (a) = arr_alloc(sizeof(*(a)), 0);                 \
    if (arr_count(a) >= arr_capacity(a))                               \
      (a) = arr_realloc((arr(void))a, capacity_grow(arr_capacity(a))); \
    (a)[arr_header(a)->count++] = (__VA_ARGS__);                       \
  } while (0)

#define arr_sort(a, cmp) qsort(a, arr_count(a), arr_item_size(a), cmp)

arr(str_t) str_split_words(str_t s);
arr(str_t) str_split_lines(str_t s);

typedef arr(char) dstr_t;

#define dstr_free(dstr) arr_free(dstr)

void dstr_append(dstr_t *ds, char ch);
void dstr_append_str(dstr_t *ds, str_t s);

typedef struct {
  usize count;
  usize capacity;
  usize item_size;
  str_t *entries;
} map_header_t;

#define map(T) T *

#define map_header(m) ((map_header_t *)m - 1)
#define map_count(m) (map_header(m)->count)
#define map_capacity(m) (map_header(m)->capacity)
#define map_item_size(m) (map_header(m)->item_size)
#define map_size(m) (map_capacity(m) * map_item_size(m))
#define map_entries(m) (map_header(m)->entries)

map(void) map_alloc(usize item_size, usize capacity);
map(void) map_realloc(map(void) m, usize new_capacity);
void map_free(map(void) m);

usize map_get_idx(map(void) m, str_t k);
void *map_get_ptr(map(void) m, str_t k);

#define map_has(m, k) (m == NULL ? false : (map_get_idx(m, k) != (usize) - 1))
#define map_get(m, k) ((typeof(m))map_get_ptr(m, k))

void map_insert_cpy(map(void) * m, str_t k, void *data);
#define map_insert(m, k, ...)                              \
  do {                                                     \
    if ((m) == NULL) (m) = map_alloc(sizeof(*(m)), 32);    \
    usize map_insert_i = map_insert_key((void **)&(m), k); \
    (m)[map_insert_i] = (__VA_ARGS__);                     \
  } while (0)

#define map_items_collect(m, dst)                                            \
  for (usize map_items_collect_i = 0; map_items_collect_i < map_capacity(m); \
       map_items_collect_i++) {                                              \
    if (map_entries(m)[map_items_collect_i].ptr == NULL) continue;           \
    typeof(*acc) pair = {map_entries(m)[map_items_collect_i],                \
                         *map_get(m, map_entries(m)[map_items_collect_i])};  \
    arr_append(dst, pair);                                                   \
  }

dstr_t read_file(cstr_t filename);

typedef struct {
  arr(cstr_t) cmd;
} cmd_t;

void cmd_free(cmd_t cmd);
void cmd_append(cmd_t *cmd, cstr_t arg);
void __cmd_append_all(cmd_t *cmd, ...);
#define cmd_append_all(pcmd, ...) __cmd_append_all(pcmd, __VA_ARGS__, NULL)
void cmd_run(cmd_t *cmd);
void cmd_run_reset(cmd_t *cmd);

#define cmd(...)                       \
  do {                                 \
    cmd_t cmd = {0};                   \
    cmd_append_all(&cmd, __VA_ARGS__); \
    cmd_run(&cmd);                     \
    cmd_free(cmd);                     \
  } while (0);

#endif  // STDR_H_

#ifdef STDR_IMPLEMENTATION

#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <strings.h>
#include <sys/wait.h>  // waitpid
#include <unistd.h>    // fork

#ifndef STDR_ASSERT
#include <assert.h>
#define STDR_ASSERT assert
#endif

usize stdr_hash(str_t k) {
  usize hash = 0;
  for (usize i = 0; i < k.len; ++i) {
    hash = ((u8 *)(k.ptr))[i] + (hash << 6) + (hash << 16) - hash;
  }
  return hash;
}

usize capacity_grow(usize capacity) {
  return capacity < 8 ? 8 : capacity + capacity / 2;
}

bool is_space(char ch) { return isspace(ch); }
bool not_is_space(char ch) { return !is_space(ch); }

bool is_new_line(char ch) { return ch == '\n'; }
bool not_is_new_line(char ch) { return !is_new_line(ch); }

str_t str_alloc(usize len) {
  str_t s = {.ptr = malloc(len), .len = len + 1};
  memset(s.ptr, 0, len + 1);
  return s;
}

void str_to_lowercase(str_t s) {
  for (usize i = 0; i < s.len; i++) {
    ((char *)s.ptr)[i] = (char)tolower(((char *)s.ptr)[i]);
  }
}

void str_to_uppercase(str_t s) {
  for (usize i = 0; i < s.len; i++) {
    ((char *)s.ptr)[i] = (char)toupper(((char *)s.ptr)[i]);
  }
}

str_t str_cpy_alloc(str_t s) {
  str_t cpy = str_alloc(s.len);
  return str_cpy(s, cpy.ptr);
}

str_t str_cpy(str_t s, void *dst) {
  str_t cpy = (str_t){.ptr = dst, .len = s.len};
  memcpy(cpy.ptr, s.ptr, s.len);
  return cpy;
}

str_t str_drop(str_t str, usize n) {
  if (str.len <= 0) return str;
  str.ptr += n;
  str.len -= n;
  return str;
}

bool str_drop_while(str_t *str, bool (*f)(char)) {
  return str_split_while(*str, NULL, str, f);
}

bool str_split_at(str_t s, str_t *lhs, str_t *rhs, usize i) {
  if (i > s.len) return false;

  if (lhs != NULL) {
    lhs->ptr = s.ptr;
    lhs->len = i;
  }

  if (rhs != NULL) {
    rhs->ptr = s.ptr + i;
    rhs->len = s.len - i;
  }

  return true;
}

bool str_split_while(str_t s, str_t *lhs, str_t *rhs, bool (*f)(char)) {
  if (s.len == 0) {
    return false;
  }

  usize i = 0;
  while (i < s.len && f(s.ptr[i])) i++;

  return str_split_at(s, lhs, rhs, i);
}

arr(str_t) str_split_words(str_t s) {
  str_t word;
  arr(str_t) words = NULL;
  while (str_split_while(s, &word, &s, not_is_space)) {
    s = str_trim_start(s);
    arr_append(words, word);
  }
  return words;
}

str_t str_trim_start(str_t s) {
  str_drop_while(&s, is_space);
  return s;
}

arr(str_t) str_split_lines(str_t s) {
  arr(str_t) lines = NULL;

  str_t line;
  while (str_split_while(s, &line, &s, not_is_new_line)) {
    s = str_drop(s, 1);
    arr_append(lines, line);
  }
  return lines;
}

void dstr_append(dstr_t *ds, char ch) { arr_append(*ds, ch); }
void dstr_append_str(dstr_t *ds, str_t s) {
  for (usize i = 0; i < s.len; i++) {
    dstr_append(ds, s.ptr[i]);
  }
}

void arr_free(arr(void) a) { STDR_FREE(arr_header(a)); }

arr(void) arr_alloc(usize item_size, usize capacity) {
  arr_header_t *a = STDR_MALLOC(sizeof(arr_header_t) + item_size * capacity);
  a->item_size = item_size;
  a->count = 0;
  a->capacity = capacity;

  return a + 1;
}

arr(void) arr_realloc(arr(void) a, usize new_capacity) {
  arr(void) b = arr_alloc(arr_item_size(a), new_capacity);
  arr_header(b)->count = arr_header(a)->count;
  memcpy(b, a, arr_item_size(a) * arr_count(a));
  arr_free(a);
  return b;
}

map(void) map_alloc(usize item_size, usize capacity) {
  map_header_t *map = malloc(sizeof(map_header_t) + capacity * item_size);
  map->count = 0;
  map->capacity = capacity;
  map->item_size = item_size;
  map->entries = malloc(capacity * sizeof(*map->entries));
  memset(map->entries, 0, capacity * sizeof(*map->entries));
  return map + 1;
}

usize map_insert_key(map(void) * m, str_t k) {
  usize i = STDR_HASH(k);
  for (usize ii = 0; ii < 32; ii++) {
    usize iii = (i + ii) % map_capacity(*m);
    if (map_entries((*m))[iii].ptr != NULL) continue;
    map_entries(*m)[iii] = k;
    map_count(*m) += 1;
    return iii;
  }

  *m = map_realloc(*m, capacity_grow(map_capacity(*m)));
  usize iii = map_insert_key(m, k);
  return iii;
}

void map_insert_cpy(map(void) * m, str_t k, void *data) {
  usize n = map_insert_key(m, k);

  u8 *ma = (u8 *)(*m);
  u8 *dest = &(ma[n * map_item_size(*m)]);

  memcpy(dest, data, map_item_size(*m));
}

map(void) map_realloc(map(void) m, usize new_capacity) {
  assert(new_capacity >= map_count(m));

  map(void) map_new = map_alloc(map_item_size(m), new_capacity);
  for (usize i = 0; i < map_capacity(m); i++) {
    if (map_entries(m)[i].ptr == NULL) continue;
    map_insert_cpy(&map_new, map_entries(m)[i],
                   map_get_ptr(m, map_entries(m)[i]));
  }
  map_free(m);
  return map_new;
}

void map_free(map(void) m) {
  if (m == NULL) return;
  free(map_header(m)->entries);
  free(map_header(m));
}

usize map_get_idx(map(void) m, str_t k) {
  usize i = STDR_HASH(k);
  for (usize ii = 0; ii < 32; ii++) {
    usize iii = (i + ii) % map_capacity(m);
    if (map_entries(m)[iii].ptr == NULL) continue;
    if (map_entries(m)[iii].len != k.len) continue;
    if (memcmp(map_entries(m)[iii].ptr, k.ptr, k.len) == 0) return iii;
  }
  return (usize)-1;
}

void *map_get_ptr(map(void) m, str_t k) {
  usize idx = map_get_idx(m, k);
  if (idx == (usize)-1) return NULL;
  return &((u8 *)m)[idx * map_item_size(m)];
}

arr(char) read_file(cstr_t filename) {
  FILE *f = fopen(filename, "r");

  arr(char) content = NULL;
  int ch;
  while (ch = fgetc(f), ch != EOF) {
    arr_append(content, (char)ch);
  }
  arr_append(content, '\0');
  fclose(f);
  return content;
}

void cmd_append(cmd_t *cmd, cstr_t arg) { arr_append(cmd->cmd, arg); }

void __cmd_append_all(cmd_t *cmd, ...) {
  va_list argptr;
  va_start(argptr, cmd);

  cstr_t a;
  while ((a = va_arg(argptr, cstr_t), a != NULL)) {
    cmd_append(cmd, a);
  }
}

void cmd_free(cmd_t cmd) { arr_free(cmd.cmd); }

void cmd_run(cmd_t *cmd) {
  assert(arr_count(cmd->cmd) >= 1 && "path must be provided.");

  printf("[CMD] ");
  for (usize i = 0; i < arr_count(cmd->cmd); i++) {
    printf("%s ", cmd->cmd[i]);
  }
  printf("\n");

  cmd_append(cmd, NULL);

  pid_t pid = fork();
  assert(pid >= 0);
  if (pid == 0) {
    cstr_t path = cmd->cmd[0];
    cstr_t *argv = &cmd->cmd[0];
    if (execvp(path, argv) < 0) {
      fprintf(stderr, "[CMD] ERROR: %s\n", strerror(errno));
      exit(1);
    }
    exit(0);
  } else {
    int status;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status)) {
      // printf("[CMD] Exited with status code %d\n", WEXITSTATUS(status));
    } else {
      printf("[CMD] Exit abnormally with code %d\n", WEXITSTATUS(status));
    }
  }

  return;
}

void cmd_run_reset(cmd_t *cmd) {
  arr_free(cmd->cmd);
  *cmd->cmd = NULL;
}

#endif  // STDR_IMPLEMENTATION
#undef STDR_IMPLEMENTATION

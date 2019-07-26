#include "9cc.h"

Vector *new_vector() {
  Vector *vec = calloc(1, sizeof(Vector));
  vec->data = malloc(sizeof(void *) * 16);
  vec->capacity = 16;
  vec->len = 0;
  return vec;
}

void vec_push(Vector *vec, void *elem) {
  if (vec->capacity == vec->len) {
    vec->capacity *= 2;
    vec->data = realloc(vec->data, sizeof(void *) * vec->capacity);
  }
  vec->data[vec->len++] = elem;
}

Map *new_map() {
  Map *map = calloc(1, sizeof(Map));
  map->keys = new_vector();
  map->vals = new_vector();
  return map;
}

void map_put(Map *map, char *key, void *val) {
  vec_push(map->keys, key);
  vec_push(map->vals, val);
}

void *map_get(Map *map, char *key) {
  for (int i = map->keys->len - 1; i >= 0; i--)
    if (strcmp(map->keys->data[i], key) == 0)
      return map->vals->data[i];
  return NULL;
}

void error(char *message, char *input) {
  fprintf(stderr, message, input);
  exit(1);
}

int expect(int line, int expected, int actual) {
  if (expected == actual) {
    return 1;
  }
  fprintf(stderr, "%d: %d expected, but got %d\n", line, expected, actual);
  exit(1);

  return 0;
}

void test_vector() {
  Vector *vec = new_vector();
  expect(__LINE__, 0, vec->len);

  for (int i = 0; i < 100; ++i) {
    int *value = malloc(sizeof(int));
    *value = i;
    vec_push(vec, (void *)value);
  }

  expect(__LINE__, 100, vec->len);
  expect(__LINE__, 0, *(int *)vec->data[0]);
  expect(__LINE__, 50, *(int *)vec->data[50]);
  expect(__LINE__, 99, *(int *)vec->data[99]);
  printf("test_vector OK\n");
}

void test_map() {
  Map *map = new_map();
  expect(__LINE__, 1, (int)(map_get(map, "foo") == NULL));

  int *value1 = malloc(sizeof(int));
  *value1 = 2;
  map_put(map, "foo", (void *)value1);
  expect(__LINE__, 2, *(int *)map_get(map, "foo"));

  int *value2 = malloc(sizeof(int));
  *value2 = 4;
  map_put(map, "bar", (void *)value2);
  expect(__LINE__, 4, *(int *)map_get(map, "bar"));

  int *value3 = malloc(sizeof(int));
  *value3 = 6;
  map_put(map, "foo", (void *)value3);
  expect(__LINE__, 6, *(int *)map_get(map, "foo"));

  printf("test_map OK\n");
}

void runtest() {
  test_vector();
  test_map();
  printf("all tests OK\n");
}

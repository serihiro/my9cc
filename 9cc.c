#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

enum {
  TK_NUM = 256,
  TK_IDENT,
  TK_EOF,
};

enum {
  ND_NUM = 256,
  ND_IDENT,
};

typedef struct {
  int ty;
  int val;
  char *input;
} Token;

typedef struct Node {
  int ty;
  struct Node *lhs;
  struct Node *rhs;
  int val;
  char *name;
} Node;

typedef struct {
  void **data;
  int capacity;
  int len;
} Vector;

typedef struct {
  Vector *keys;
  Vector *vals;
} Map;

Vector *tokens;
int pos;

Node *code[100];
Map *variable_map;
int variable_offset;

void error(char *message, char *input);
int consume(int ty);
void tokenize(char *p);
void gen(Node *node);
void gen_lval(Node *node);
int expect(int line, int expected, int actual);
void runtest();
Node *new_node(int ty, Node *lhs, Node *rhs);
Node *new_node_num(int val);
Node *new_node_ident(char *name);
Node *add();
Node *mul();
Node *term();
Vector *new_vector();
void vec_push(Vector *vec, void *elem);
void program();
Node *stmt();
Node *assign();
Map *new_map();
void map_put(Map *map, char *key, void *val);
void *map_get(Map *map, char *key);

Token *new_token() {
  Token *token = malloc(sizeof(Token));
  return token;
}

Node *new_node(int ty, Node *lhs, Node *rhs) {
  Node *node = malloc(sizeof(Node));
  node->ty = ty;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int val) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_NUM;
  node->val = val;
  return node;
}

Node *new_node_ident(char *name) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_IDENT;
  node->name = name;
  return node;
}

int consume(int ty) {
  Token *token = (Token *)tokens->data[pos];
  if (token->ty != ty) {
    return 0;
  }
  ++pos;
  return 1;
}

Node *term() {
  Token *token = (Token *)tokens->data[pos];
  if (consume('(')) {
    Node *node = add();

    if (!consume(')')) {
      error("開きカッコに対応する閉じカッコがありません: %s", token->input);
      return node;
    }

    return node;
  }

  if (token->ty == TK_NUM) {
    ++pos;
    return new_node_num(token->val);
  }

  if (token->ty == TK_IDENT) {
    ++pos;
    char *variable = (char *)malloc(sizeof(char) * (strlen(token->input) + 1));
    strcpy(variable, token->input);
    return new_node_ident(variable);
  }

  error("数値でも変数でも開きカッコでもないトークンです: %s", token->input);

  return NULL;
}

Node *mul() {
  Node *node = term();

  for (;;) {
    if (consume('*')) {
      node = new_node('*', node, term());
    } else if (consume('/')) {
      node = new_node('/', node, term());
    } else {
      return node;
    }
  }
}

Node *add() {
  Node *node = mul();

  for (;;) {
    if (consume('+')) {
      node = new_node('+', node, mul());
    } else if (consume('-')) {
      node = new_node('-', node, mul());
    } else {
      return node;
    }
  }
}

Node *assign() {
  Node *node = add();

  for (;;) {
    if (consume('=')) {
      node = new_node('=', node, assign());
    } else {
      return node;
    }
  }
}

Node *stmt() {
  Node *node = assign();
  if (!consume(';')) {
    Token *token = (Token *)tokens->data[pos];
    error("';'ではないトークンです: %s", token->input);
  }

  return node;
}

void program() {
  int i = 0;
  Token *token = (Token *)tokens->data[pos];
  while (token->ty != TK_EOF) {
    code[i++] = stmt();
    token = (Token *)tokens->data[pos];
  }
  code[i] = NULL;
}

Vector *new_vector() {
  Vector *vec = malloc(sizeof(Vector));
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
  Map *map = malloc(sizeof(Map));
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

void gen_lval(Node *node) {
  if (node->ty != ND_IDENT) {
    error("代入の左辺値が変数ではありません:%c", (char *)&node->val);
  }

  printf("  mov rax, rbp\n");

  // calculate address of the target variable
  int *offset = (int *)map_get(variable_map, node->name);
  if (offset == NULL) {
    // varibale size if fixed 8 Byte, for now
    variable_offset += 8;
    offset = (int *)malloc(sizeof(int));
    *offset = variable_offset;
    char *variable_name = (char *)malloc(sizeof(char) * (strlen(node->name) + 1));
    strcpy(variable_name, node->name);
    map_put(variable_map, variable_name, offset);
  }
  printf("  sub rax, %d\n", *offset);
  printf("  push rax\n"); // push the address of the target variable
}

void gen(Node *node) {
  if (node->ty == ND_NUM) {
    printf("  push %d\n", node->val);
    return;
  }

  if (node->ty == ND_IDENT) {
    gen_lval(node);
    printf("  pop rax\n");
    printf("  mov rax, [rax]\n");
    printf("  push rax\n");
    return;
  }

  if (node->ty == '=') {
    gen_lval(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  mov [rax], rdi\n");
    printf("  push rdi\n");
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->ty) {
  case '+':
    printf("  add rax, rdi\n");
    break;
  case '-':
    printf("  sub rax, rdi\n");
    break;
  case '*':
    printf("  mul rdi\n");
    break;
  case '/':
    printf("  mov rdx, 0\n");
    printf("  div rdi\n");
  }

  printf("  push rax\n");
}

void tokenize(char *p) {
  while (*p) {
    if (isspace(*p)) {
      ++p;
      continue;
    }

    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' ||
        *p == ')' || *p == ';' || *p == '=') {
      Token *token = new_token();
      token->ty = *p;
      token->input = p;
      vec_push(tokens, token);
      ++p;
      continue;
    }

    if (isdigit(*p)) {
      Token *token = new_token();
      token->ty = TK_NUM;
      token->input = p;
      token->val = strtol(p, &p, 10);
      vec_push(tokens, token);
      continue;
    }

    // name of variable must start with alphabet
    if (isalpha(*p)) {
      Token *token = new_token();
      token->ty = TK_IDENT;

      int str_len = 0;
      while (isalnum(*(p + str_len)))
        ++str_len;
      char * variable = (char *)malloc(sizeof(char) * str_len + 1);
      strncpy(variable, p, str_len);
      variable[str_len] = '\0';

      p += str_len;
      token->input = variable;
      vec_push(tokens, token);
      continue;
    }

    error("トークナイズできません: %s\n", p);
  }

  Token *token = new_token();
  token->ty = TK_EOF;
  token->input = p;
  vec_push(tokens, token);
}

void error(char *message, char *input) {
  fprintf(stderr, message, input);
  exit(1);
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }

  if (strcmp(argv[1], "-test") == 0) {
    runtest();
    return 0;
  }

  tokens = new_vector();
  variable_map = new_map();

  tokenize(argv[1]);
  program();

  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  // allocate stack spaces for 26 variables
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, 208\n");

  for (int i = 0; code[i] != NULL; i++) {
    gen(code[i]);
    printf("  pop rax\n");
  }

  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
  return 0;
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


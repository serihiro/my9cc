#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

enum {
  TK_NUM = 256,
  TK_EQ,
  TK_NEQ,
  TK_IDENT,
  TK_EOF,
  TK_RETURN,
  TK_IF,
  TK_ELSE,
  TK_WHILE,
  TK_FOR,
  ND_NUM,
  ND_IDENT,
  ND_EQ,
  ND_NEQ,
  ND_CALL,
  ND_RETURN,
  ND_IF,
  ND_WHILE,
  ND_FOR,
};

typedef struct {
  int ty;
  int val;
  char *input;
  int len;
} Token;

typedef struct {
  void **data;
  int capacity;
  int len;
} Vector;

typedef struct Node {
  int ty;
  struct Node *lhs;
  struct Node *rhs;
  struct Node *els;
  struct Node *inner;
  int val;
  char *name;
  Vector *args;
  int offset;
} Node;

typedef struct {
  Vector *keys;
  Vector *vals;
} Map;

typedef struct LVar {
  struct LVar *next;
  char *name;
  int len;
  int offset;
} LVar;

extern Vector *tokens;
extern int pos;
extern Node *code[100];
extern LVar *locals;
extern int seq_if;
extern int seq_while;
extern int seq_for;

void error(char *message, char *input);
int consume(int ty);
void tokenize(char *p);
void gen(Node *node);
void gen_lval(Node *node);
int expect(int line, int expected, int actual);
void runtest();
Node *new_node(int ty, Node *lhs, Node *rhs);
Node *new_node_num(int val);
Node *new_node_ident(char *name, int offset);
Node *new_node_call(char *name, Vector *args);
Node *add();
Node *mul();
Node *comp();
Node *term();
Vector *new_vector();
void vec_push(Vector *vec, void *elem);
void program();
Node *stmt();
Node *assign();
Map *new_map();
void map_put(Map *map, char *key, void *val);
void *map_get(Map *map, char *key);

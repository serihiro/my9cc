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

extern Vector *tokens;
extern int pos;
extern Node *code[100];
extern Map *variable_map;
extern int variable_offset;

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

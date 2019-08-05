#include "9cc.h"

Vector *tokens;
int pos = 0;
Node *code[100];
LVarList *locals;
int seq_if = 0;
int seq_while = 0;
int seq_for = 0;

void push_locals() {
  LVarList *new_var = calloc(1, sizeof(LVarList));
  new_var->lvar = calloc(1, sizeof(LVar));
  new_var->next = locals;
  locals = new_var;
}

void pop_locals() { locals = locals->next; }

LVar *find_lvar(Token *token) {
  for (LVar *var = locals->lvar; var; var = var->next)
    if (var->len == token->len && !memcmp(token->input, var->name, var->len))
      return var;
  return NULL;
}

Token *new_token() {
  Token *token = calloc(1, sizeof(Token));
  return token;
}

Node *new_node(int ty, Node *lhs, Node *rhs) {
  Node *node = calloc(1, sizeof(Node));
  node->ty = ty;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int val) {
  Node *node = calloc(1, sizeof(Node));
  node->ty = ND_NUM;
  node->val = val;
  return node;
}

Node *new_node_ident(char *name, int offset) {
  Node *node = calloc(1, sizeof(Node));
  node->ty = ND_IDENT;
  node->name = name;
  node->offset = offset;
  return node;
}

Node *new_node_call(char *name, Vector *args) {
  Node *node = calloc(1, sizeof(Node));
  node->ty = ND_CALL;
  node->name = name;
  node->args = args;

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
      return NULL;
    }

    return node;
  }

  if (token->ty == TK_NUM) {
    ++pos;
    return new_node_num(token->val);
  }

  if (token->ty == TK_IDENT) {
    ++pos;
    if (!consume('(')) {

      LVar *lvar = find_lvar(token);
      if (!lvar) {
        lvar = calloc(1, sizeof(LVar));
        lvar->next = locals->lvar;
        lvar->name = token->input;
        lvar->len = token->len;
        lvar->offset = locals->lvar->offset + 8;
        locals->lvar = lvar;
      }

      return new_node_ident(token->input, lvar->offset);
    }

    Vector *args = new_vector();
    if (consume(')')) {
      return new_node_call(token->input, args);
    }

    vec_push(args, assign());
    while (consume(',')) {
      vec_push(args, assign());
    }

    if (!consume(')')) {
      error("開きカッコに対応する閉じカッコがありません: %s", token->input);
      return NULL;
    }

    return new_node_call(token->input, args);
  }

  error("数値でも変数でも開きカッコでも関数呼び出しでもないトークンです: %s",
        token->input);

  return NULL;
}

Node *comp() {
  Node *node = term();

  for (;;) {
    if (consume(TK_EQ)) {
      node = new_node(ND_EQ, node, term());
    } else if (consume(TK_NEQ)) {
      node = new_node(ND_NEQ, node, term());
    } else {
      return node;
    }
  }
}

Node *mul() {
  Node *node = comp();

  for (;;) {
    if (consume('*')) {
      node = new_node('*', node, comp());
    } else if (consume('/')) {
      node = new_node('/', node, comp());
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
  Node *node;

  if (consume(TK_RETURN)) {
    node = calloc(1, sizeof(Node));
    node->ty = ND_RETURN;
    node->lhs = assign();
  } else if (consume(TK_IF) && consume('(')) {
    node = calloc(1, sizeof(Node));
    node->ty = ND_IF;
    // This statement is `A` of `if(A) B;`
    node->lhs = assign();
    if (!consume(')')) {
      Token *token = (Token *)tokens->data[pos];
      error("')'ではないトークンです: %s", token->input);
    }

    // This statement is `B` of `if(A) B;`
    node->rhs = stmt();
    if (consume(TK_ELSE)) {
      node->els = stmt();
    }

    return node;
  } else if (consume(TK_WHILE) && consume('(')) {
    node = calloc(1, sizeof(Node));
    node->ty = ND_WHILE;

    // This statement is `A` of `while(A) B;`
    node->lhs = assign();
    if (!consume(')')) {
      Token *token = (Token *)tokens->data[pos];
      error("')'ではないトークンです: %s", token->input);
    }
    // This statement is `B` of `while(A) B;`
    node->rhs = stmt();

    return node;
  } else if (consume(TK_FOR) && consume('(')) {
    node = calloc(1, sizeof(Node));
    node->ty = ND_FOR;

    // This expr is `A of `for(A;B;C)D;`
    if (!consume(';')) {
      node->lhs = assign();
      if (!consume(';')) {
        Token *token = (Token *)tokens->data[pos];
        error("';'ではないトークンです: %s", token->input);
      }
    }

    // This expr is `B of `for(A;B;C)D;`
    if (!consume(';')) {
      node->rhs = assign();
      if (!consume(';')) {
        Token *token = (Token *)tokens->data[pos];
        error("';'ではないトークンです: %s", token->input);
      }
    }

    // This expr is `C of `for(A;B;C)D;`
    if (!consume(')')) {
      node->els = assign();
      if (!consume(')')) {
        Token *token = (Token *)tokens->data[pos];
        error("')'ではないトークンです: %s", token->input);
      }
    }

    // This stmt is `D of `for(A;B;C)D;`
    node->inner = stmt();

    return node;
  } else if (consume('{')) {
    node = calloc(1, sizeof(Node));
    node->ty = ND_BLOCK;
    push_locals();

    Vector *lines = new_vector();
    while (!consume('}')) {
      vec_push(lines, stmt());
    }
    node->args = lines;

    pop_locals();
    return node;
  } else {
    node = assign();
  }

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

void tokenize(char *p) {
  while (*p) {
    if (isspace(*p)) {
      ++p;
      continue;
    }

    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' ||
        *p == ')' || *p == ';' || *p == ',' || *p == '{' || *p == '}') {
      Token *token = new_token();
      token->ty = *p;
      token->input = p;
      vec_push(tokens, token);
      ++p;
      continue;
    }

    if (*p == '=') {
      Token *token = new_token();
      if (*(p + 1) != '\0' && *(p + 1) == '=') {
        token->ty = TK_EQ;
        token->input = p;
        vec_push(tokens, token);
        p += 2;
      } else {
        token->ty = *p;
        token->input = p;
        vec_push(tokens, token);
        ++p;
      }
      continue;
    }

    if (*p == '!') {
      if (*(p + 1) != '\0' && *(p + 1) == '=') {
        Token *token = new_token();
        token->ty = TK_NEQ;
        token->input = p;
        vec_push(tokens, token);
        p += 2;
        continue;
      }
    }

    if (isdigit(*p)) {
      Token *token = new_token();
      token->ty = TK_NUM;
      token->input = p;
      token->val = strtol(p, &p, 10);
      vec_push(tokens, token);
      continue;
    }

    // name of variable and function must start with alphabet
    if (isalpha(*p)) {
      Token *token = new_token();

      int str_len = 0;
      while (isalnum(*(p + str_len)))
        ++str_len;

      char *variable = (char *)malloc(sizeof(char) * (str_len + 1));
      strncpy(variable, p, str_len);
      variable[str_len] = '\0';

      token->input = variable;
      token->len = str_len;
      if (strncmp(variable, "return", 6) == 0 && !(isalpha(p[6]))) {
        token->ty = TK_RETURN;
      } else if (strncmp(variable, "if", 2) == 0 && !(isalpha(p[2]))) {
        token->ty = TK_IF;
      } else if (strncmp(variable, "else", 4) == 0 && !(isalpha(p[4]))) {
        token->ty = TK_ELSE;
      } else if (strncmp(variable, "while", 5) == 0 && !(isalpha(p[5]))) {
        token->ty = TK_WHILE;
      } else if (strncmp(variable, "for", 3) == 0 && !(isalpha(p[3]))) {
        token->ty = TK_FOR;
      } else {
        token->ty = TK_IDENT;
      }

      vec_push(tokens, token);
      p += token->len;
      continue;
    }

    error("トークナイズできません: %s\n", p);
  }

  Token *token = new_token();
  token->ty = TK_EOF;
  token->input = p;
  vec_push(tokens, token);
}

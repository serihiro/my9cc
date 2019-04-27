#include "9cc.h"

Vector *tokens;
int pos;
Node *code[100];
Map *variable_map;
int variable_offset;

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

Node *new_node_num(int *val) {
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

Node *new_node_call(char *name, int *args) {
  Node *node = malloc(sizeof(Node));
  node->ty = ND_CALL;
  node->name = name;
  node->val = args;
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

  if (token->ty == TK_CALL) {
    ++pos;
    char *variable = (char *)malloc(sizeof(char) * (strlen(token->input) + 1));
    strcpy(variable, token->input);
    return new_node_call(variable, token->val);
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

void tokenize(char *p) {
  while (*p) {
    if (isspace(*p)) {
      ++p;
      continue;
    }

    if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' ||
        *p == ')' || *p == ';') {
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
      int *val = (int *)malloc(sizeof(int));
      *val = strtol(p, &p, 10);
      token->val = val;
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

      if (*(p + str_len) != '\0' && *(p + str_len) == '(') {
        p += (str_len + 1);
        int *function_args = malloc(sizeof(int) * 6);

        int function_args_offset = 0;
        bool expect_comma = false;
        while (*p != ')') {
          if (isspace(*p)) {
            ++p;
            continue;
          }

          if (isdigit(*p)) {
            if (expect_comma)
              error("カンマを想定していましたが数字ですねこれ．:%s\n", p);
            if (function_args_offset == 5)
              error("関数の引数は最大6個です．:%s\n", p);

            function_args[function_args_offset++] = strtol(p, &p, 10);
            expect_comma = true;
            continue;
          }

          if (*p == ',') {
            if (!expect_comma)
              error("数値を想定していましたがカンマですねこれ．:%s\n", p);

            ++p;
            expect_comma = false;
            continue;
          }

          error("想定していない文字が引数として渡されてますねこれ．:%s\n", p);
        }

        if (function_args_offset != 0 && !expect_comma)
          error("関数の引数リストの最後がカンマで終わってますねこれ多分．:%s\n",
                p);

        // for closing braces
        ++p;

        token->val = function_args;
        token->ty = TK_CALL;
      } else {
        token->ty = TK_IDENT;
        p += str_len;
      }

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

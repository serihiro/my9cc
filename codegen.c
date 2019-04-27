#include "9cc.h"

char *args_register_name_by_order(int order) {
  char *result = "";

  switch (order) {
  case 0:
    result = "rdi";
    break;
  case 1:
    result = "rsi";
    break;
  case 2:
    result = "rdx";
    break;
  case 3:
    result = "rcx";
    break;
  case 4:
    result = "r8";
    break;
  case 5:
    result = "r9";
  }

  return result;
}

void gen_lval(Node *node) {
  if (node->ty != ND_IDENT) {
    error("代入の左辺値が変数ではありません", "");
  }

  printf("  mov rax, rbp\n");

  // calculate address of the target variable
  int *offset = (int *)map_get(variable_map, node->name);
  if (offset == NULL) {
    // varibale size if fixed 8 Byte, for now
    variable_offset += 8;
    offset = (int *)malloc(sizeof(int));
    *offset = variable_offset;
    char *variable_name =
        (char *)malloc(sizeof(char) * (strlen(node->name) + 1));
    strcpy(variable_name, node->name);
    map_put(variable_map, variable_name, offset);
  }
  printf("  sub rax, %d\n", *offset);
  printf("  push rax\n"); // push the address of the target variable
}

void gen(Node *node) {
  if (node->ty == ND_NUM) {
    printf("  push %d\n", node->val[0]);
    return;
  }

  if (node->ty == ND_IDENT) {
    gen_lval(node);
    printf("  pop rax\n");
    printf("  mov rax, [rax]\n");
    printf("  push rax\n");
    return;
  }

  if (node->ty == ND_CALL) {
    for (int i = 0; i < 6; ++i) {
      printf("  mov %s, %d\n", args_register_name_by_order(i), node->val[i]);
    }
    printf("  call %s\n", node->name);
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
    break;
  case ND_EQ:
    printf("  cmp rdi, rax\n");
    printf("  sete al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_NEQ:
    printf("  cmp rdi, rax\n");
    printf("  setne al\n");
    printf("  movzb rax, al\n");
    break;
  }

  printf("  push rax\n");
}

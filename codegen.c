#include "9cc.h"

const char *ARG_REGS[6] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

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

  if (node->ty == ND_CALL) {
    printf("  push rbx\n");
    printf("  push rbp\n");
    printf("  push rsp\n");
    printf("  push r12\n");
    printf("  push r13\n");
    printf("  push r14\n");
    printf("  push r15\n");
    for (int i = 0; i < 6; ++i) {
      if (node->args->len > i) {
        gen((Node *)node->args->data[i]);
        printf("  pop rax\n");
        printf("  mov %s, rax\n", ARG_REGS[i]);
      } else {
        printf("  mov %s, %d\n", ARG_REGS[i], 0);
      }
    }
    printf("  call %s\n", node->name);
    printf("  pop r15\n");
    printf("  pop r14\n");
    printf("  pop r13\n");
    printf("  pop r12\n");
    printf("  pop rsp\n");
    printf("  pop rbp\n");
    printf("  pop rbx\n");

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

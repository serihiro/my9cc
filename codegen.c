#include "9cc.h"

const char *ARG_REGS[6] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

void gen_lval(Node *node) {
  if (node->ty != ND_IDENT) {
    error("代入の左辺値が変数ではありません:%c", (char *)&node->val);
  }

  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", node->offset);
  printf("  push rax\n"); // push the address of the target variable
}

void gen(Node *node) {
  if (node->ty == ND_BLOCK) {
    for (int i = 0; i < node->args->len; ++i) {
      gen((Node *)node->args->data[i]);
    }
    return;
  }

  if (node->ty == ND_WHILE) {
    int seq = seq_while++;
    printf(".LWhilebegin%d:\n", seq);
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je  .LWhileend%d\n", seq);
    gen(node->rhs);
    printf("  jmp .LWhilebegin%d\n", seq);
    printf(".LWhileend%d:\n", seq);
    return;
  }

  if (node->ty == ND_IF) {
    gen(node->lhs);
    int seq = seq_if++;
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");

    if (node->els != NULL) {
      printf("  je  .Lelse%d\n", seq);
      gen(node->rhs);
      printf("  jmp  .LIfend%d\n", seq);
      printf(".Lelse%d:\n", seq);
      gen(node->els);
      printf(".LIfend%d:\n", seq);
    } else {
      printf("  je  .LIfend%d\n", seq);
      gen(node->rhs);
      printf(".LIfend%d:\n", seq);
    }

    return;
  }

  if (node->ty == ND_FOR) {
    if (node->lhs != NULL) {
      gen(node->lhs);
    }
    int seq = seq_for++;
    printf(".LForbegin%d:\n", seq);
    if (node->rhs != NULL) {
      gen(node->rhs);
    }
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je  .LForend%d\n", seq);
    gen(node->inner);
    if (node->els != NULL) {
      gen(node->els);
    }
    printf("  jmp .LForbegin%d\n", seq);
    printf(".LForend%d:\n", seq);
    return;
  }

  if (node->ty == ND_RETURN) {
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return;
  }

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

#include <stdio.h>

int foo() {
  printf("foo\n");
  return 0;
}

int bar(int a, int b) {
  printf("%d\n", a + b);
  return 0;
}

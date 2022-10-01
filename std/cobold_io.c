#include <malloc/_malloc.h>
#include <stdint.h>
#include <stdio.h>

struct __lib_string {
  int64_t size;
  int8_t *data;
};

void Print(int x) { printf("%d\n", x); }

void PrintStr(struct __lib_string str) {
  printf("%.*s", (int)str.size, str.data);
}

void *__lib_malloc(int64_t size) { return malloc(size); }
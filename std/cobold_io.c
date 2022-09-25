#include <malloc/_malloc.h>
#include <stdint.h>
#include <stdio.h>

void Print(int x) { printf("%d\n", x); }

void *__lib_malloc(int64_t size) { return malloc(size); }
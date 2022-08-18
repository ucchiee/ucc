#include "rc_gc.h"

#include <stdlib.h>

// #define POC

int* rc_malloc(int size) {
  int* tmp = (int*)malloc(size);
  set_count(&tmp, 0);
  return tmp;
}

void rc_free(int* ptr) {
  delete_count(&ptr);
  free(ptr);
}

void rc_delete(int** ptr) {
  if (get_count(*ptr) == 0) return;
  dec_count(ptr);
  if (get_count(*ptr) == 0) {
    // TODO: Need to free children.
    // How to identify children?
    rc_free(*ptr);
  }
}

void rc_update(int** dst, int** src) {
  rc_delete(dst);
  inc_count(src);
}

// Helper functions

short get_count(int* ptr) {
  // Upper 16bit is a counter.
  return (unsigned long)ptr >> 48;
}

void set_count(int** ptr, short count) {
  // Create mask
  unsigned long next_mask = (unsigned long)count << 48;

  // Clear upper 16bit.
  unsigned long addr = (unsigned long)*ptr;
  addr = (addr << 16) >> 16;

  // Update
  *ptr = (int*)(addr | next_mask);
}

void dec_count(int** ptr) { set_count(ptr, get_count(*ptr) - 1); }
void inc_count(int** ptr) { set_count(ptr, get_count(*ptr) + 1); }

void delete_count(int** ptr) {
  if ((unsigned long)*ptr & 0x0000800000000000) {
    *ptr = (int*)((unsigned long)*ptr | 0xffff000000000000);
  } else {
    *ptr = (int*)((unsigned long)*ptr & 0x0000ffffffffffff);
  }
}

#ifdef POC
int main(int argc, char* argv[]) {
  int *mptr, *tmp;
  for (;;) {
    tmp = mptr;
    mptr = (int*)rc_malloc(1024);
    rc_update(&tmp, &mptr);
  }
  return 0;
}
#endif /* POC */

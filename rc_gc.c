#include "rc_gc.h"

#include <stdlib.h>

// #define POC

void* rc_malloc(int size) {
  void* tmp = (void*)malloc(size + 4);
  set_count(tmp, 0);
  return make_defined(tmp);
}

void rc_free(void* ptr) { free(delete_flag(ptr)); }

void rc_delete(void* ptr) {
  if (!is_defined(ptr)) return;
  dec_count(ptr);
  if (get_count(ptr) == 0) {
    // TODO: Need to free children.
    // How to identify children?
    rc_free(ptr);
  }
}

void rc_update(void** dst, void* src) {
  rc_delete(*dst);
  inc_count(src);
  *dst = src;
}

// Helper functions

short get_count(void* ptr) {
  // Upper 16bit is a counter.
  return *((int*)delete_flag(ptr));
}

void set_count(void* ptr, int c) {
  int* ref_count = (int*)delete_flag(ptr);
  *ref_count = c;
}

void dec_count(void* ptr) { set_count(ptr, get_count(ptr) - 1); }
void inc_count(void* ptr) { set_count(ptr, get_count(ptr) + 1); }

void* make_defined(void* ptr) {
  unsigned long addr = (unsigned long)ptr;
  addr = addr & 0x0000ffffffffffff;
  addr = addr + 0x0001000000000000;
  return (void*)addr;
}

int is_defined(void* ptr) {
  unsigned long addr = (unsigned long)ptr;
  return (addr & 0xffff000000000000) == 0x0001000000000000;
}

void* delete_flag(void* ptr) {
  long addr = (long)ptr;
  addr = addr << 16;
  addr = addr >> 16;
  return (void*)addr;
}

#ifdef POC
int main(int argc, char* argv[]) {
  void *mptr, *tmp;
  for (;;) {
    rc_update(&mptr, rc_malloc(1024));
  }
  return 0;
}
#endif /* POC */

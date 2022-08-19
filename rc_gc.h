#pragma once

// Interface
void* rc_malloc(int size);

// Inner function
void rc_free(void* ptr);
void rc_delete(void* ptr);
void rc_update(void** dst, void* src);

// helper function
short get_count(void* ptr);
void set_count(void* ptr, int count);
void dec_count(void* ptr);
void inc_count(void* ptr);
void* make_defined(void* ptr);
int is_defined(void* ptr);
void* delete_flag(void* ptr);

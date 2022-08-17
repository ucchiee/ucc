#pragma once

// Interface
int* rc_malloc(int size);

// Inner function
void rc_free(int* ptr);
void rc_delete(int** ptr);
void rc_update(int** dst, int** src);

// helper function
short get_count(int* ptr);
void set_count(int** ptr, short count);
void dec_count(int** ptr);
void inc_count(int** ptr);
void delete_count(int** ptr);

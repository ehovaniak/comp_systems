#ifndef NU_MEM_H
#define NU_MEM_H

#include <stdint.h>
#include <stdbool.h>

/*
typedef struct node_t {
	//bool free;
	int64_t size;
	struct node_t* next;
	//struct node_t* prev;

} node;

typedef struct header_t {
	//bool free;
	int64_t size;
} header;

typedef struct footer_t {
	int64_t size;
} footer;
*/
void* nu_malloc(size_t size);
void  nu_free(void* ptr);
void  nu_mem_print_stats();
void  nu_print_free_list();

//uint64_t next_power_of_two(uint64_t num);
//void print_free_list(void);
//static void coalesce(void);
//static void free_list_insert(node* cell);
//static node* get_free_block(int64_t size);

#endif

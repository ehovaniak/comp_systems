#include <stdint.h>
#include <sys/mman.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "nu_mem.h"

static const int64_t CHUNK_SIZE = 65536;

// You should update these counters on memory allocation / deallocation events.
// These counters should only go up, and should provide totals for the entire
// execution of the program.
static int64_t nu_malloc_count  = 0; // How many times has malloc returned a block.
static int64_t nu_malloc_bytes  = 0; // How many bytes have been allocated total
static int64_t nu_free_count    = 0; // How many times has free recovered a block.
static int64_t nu_free_bytes    = 0; // How many bytes have been recovered total.
static int64_t nu_malloc_chunks = 0; // How many chunks have been mmapped?
static int64_t nu_free_chunks   = 0; // How many chunks have been munmapped?

/* Structure for free block data */
typedef struct node_t {
	//bool free;
	int64_t size;
	struct node_t* next;
	//struct node_t* prev;

} node;

/* structure for allocated data */
typedef struct header_t {
	//bool free;
	int64_t size;
} header;

/* structure for foot of allocated data
 * Not used
 */
typedef struct footer_t {
	int64_t size;
} footer;

static void free_list_insert(node* cell);
static void coalesce(void);
void print_free_list(void);


static const int64_t NODE_SIZE = (int64_t)sizeof(node);
static const int64_t HEADER_SIZE = (int64_t)sizeof(header);

static node* free_list_head = NULL;

/*
 * calcules floor multiple of num1 / num2
 */
uint64_t multiple(uint64_t num1, uint64_t num2) {
	return 1 + ((num1 - 1) / num2);
}

/*
 * Debug function for printing the free list
 */
void print_free_list(void) {
	node* curr = free_list_head;
	while (curr) {
		printf("NODE = %d\n", curr);
		printf("NODE->SIZE = %d\n", curr->size);
		curr = curr->next;
	}
	return;
}

/*
 * Calculate length of free list by iterating through list and counting as you go.
 */
int64_t
nu_free_list_length()
{
    // TODO: This should return how many blocks of already allocated memory
    //   you have available for future malloc requests.
	node* curr = free_list_head;
	int count = 0;
	while (curr) {
		curr=curr->next;
		count++;
	}

    return count;
}

void
nu_mem_print_stats()
{
    fprintf(stderr, "\n== nu_mem stats ==\n");
    fprintf(stderr, "malloc count: %ld\n", nu_malloc_count);
    fprintf(stderr, "malloc bytes: %ld\n", nu_malloc_bytes);
    fprintf(stderr, "free count: %ld\n", nu_free_count);
    fprintf(stderr, "free bytes: %ld\n", nu_free_bytes);
    fprintf(stderr, "malloc chunks: %ld\n", nu_malloc_chunks);
    fprintf(stderr, "free chunks: %ld\n", nu_free_chunks);
    fprintf(stderr, "free list length: %ld\n", nu_free_list_length());
}


/*
 * Find a free block within the free list.
 * If a large enough block is found, return a pointer to the block. Adjust pointers within list
 * to keep free list intact
 *
 * If no free block large enough is found, return NULL
 *
 * Operates using 'first fit' method
 */
static node* get_free_block(int64_t size) {
	node* curr = free_list_head;
	node* prev = free_list_head;

	// Iterate through list
	while (curr) {
		// If large enough, get cell
		if ((curr->size + NODE_SIZE) >= size) {
			// If cell is start, adjust head, else remove from list
			if (curr == free_list_head)
				free_list_head = curr->next;
			else
				prev->next = curr->next;

			return curr;
		}
		prev = curr;
		curr = curr->next;
	}

	return NULL;
}

void*
nu_malloc(size_t usize)
{
    // TODO: Make this allocate memory.
    //
    // Allocate small blocks of memory by allocating 64k chunks
    // and then satisfying multiple requests from that.
    //
    // Allocate large blocks (>= 64k) of memory directly with
    // mmap.

	// Exception handling
	if (!usize)
		return NULL;
	else if (usize < 0)
		return -1;

	// calculate required size for requested block
	int64_t req_size = (int64_t)usize;
	int64_t req_block_size = (int64_t)usize + HEADER_SIZE;

	// Error handling for too small a size
	if (req_block_size < NODE_SIZE)
		req_block_size = NODE_SIZE;

	// Get a free block for the allocation
	node* req_block = get_free_block(req_block_size);

	// If no free block found, get a new chunk
	if (!req_block) {
		// Calculate page_size based on multiples of chunk size. Round up
		int64_t page_size = multiple(req_size + NODE_SIZE, CHUNK_SIZE) * CHUNK_SIZE;

		// get address of new page
		req_block = mmap(NULL, page_size,
				PROT_READ | PROT_WRITE,
				MAP_ANON | MAP_PRIVATE, -1, 0);
		// Set the size of the free block
		req_block->size = page_size - NODE_SIZE;
		nu_malloc_chunks++;
	}

	// Calculate how much space remains after requested size
	int64_t remainder = req_block->size - req_block_size;

	// If enough data remains to make a new 'free' block, do so
	// Else, simply give user entire space
	if (remainder >= NODE_SIZE) {
		node* free_block = (node*) ((void*)req_block + req_block_size);
		free_block->size = remainder;

		free_list_insert(free_block);
		coalesce();
	}
	else
		req_block_size += remainder;

	// Cast the requested block to a header, assign the size of the allocated block
	header* h = (header*) req_block;
	h->size = req_block_size - HEADER_SIZE;

	nu_malloc_bytes += req_block_size;
	nu_malloc_count++;

	// cast to void pointer, increment to correct address, return
	return ((void*)h + HEADER_SIZE);
}

/*
 * Insert 'cell' into free list based on address of pointer in heap
 * If free list is empty or 'cell' comes before the head, make this the start
 */
static void free_list_insert(node* cell) {

	node* block = free_list_head;
	uint64_t addr = (uint64_t)cell;

	// if there is no free_list_head, or the head is greater than address,
	// insert to start of free list
	if (!block ||
			((uint64_t)block) > addr) {
		cell->next = free_list_head;
		free_list_head = cell;
		return;
	}

	// Iterate through free list. Stops when the next block in free list
	// has an address greater than 'cell'
	// This keeps cells in the list ordered by address
	// Facilitates coalesce
	while ((block->next) && ((uint64_t)(block->next)) < addr)
		block = block->next;

	// Insert into list
	cell->next = block->next;
	block->next = cell;
}

/*
 * Merge adjacent free cells into one larger free cell. Reduces fragmentation and allows
 * program to release memory back to system
 *
 *
 */
static void coalesce(void) {
	node* block = free_list_head;

	// Iterate through free list
	while (block && block->next) {
		int64_t addr1 = (int64_t)block;
		int64_t addr2 = (int64_t)block->next;

		// Determine if blocks are adjacent
		// If so, then merge
		if (addr1+block->size+NODE_SIZE == addr2) {
			block->size += (block->next)->size + NODE_SIZE;
			block->next = (block->next)->next;
		}

		block = block->next;
	}

	return;
}

void
nu_free(void* addr) 
{
    // TODO: Make this free memory.
    //
    // Free small blocks by saving them for reuse.
    //   - Stick together adjacent small blocks into bigger blocks.
    //   - Advanced: If too many full chunks have been freed (> 4 maybe?)
    //     return some of them with munmap.
    // Free large blocks with munmap.
	//printf("== FREE ==\n");

	// Get address of requested block and cast to a node
	header* req_block = (header*)(addr - HEADER_SIZE);
	int64_t req_block_size = req_block->size + HEADER_SIZE;
	node* free_block  = (node*)req_block;

	// If size is larger than a chunk, then release a page of size n*CHUNK_SIZE to system
	if (req_block_size >= CHUNK_SIZE) {
		// calculate floor size for page release. munmap only operates on multiples of a page size
		// Prevents unmap from from removing address still in use.
		int64_t reduction = (req_block_size / CHUNK_SIZE) * CHUNK_SIZE;
		// Release the page
		munmap((void*)free_block, reduction);
		nu_free_chunks++;

		// Locate the remaining memory after unmap and set size
		free_block = (node*) ((void*)free_block + reduction);
		free_block->size = req_block_size - reduction - NODE_SIZE;
	}
	else
		free_block->size = req_block_size - NODE_SIZE;

	// Insert newly created free block into list and coalesce
	free_list_insert(free_block);
	coalesce();
	nu_free_bytes += req_block_size;
	nu_free_count++;

	return;
}























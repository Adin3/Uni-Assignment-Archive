// SPDX-License-Identifier: BSD-3-Clause
#define METADATA_SIZE		(sizeof(struct block_meta))
#define MOCK_PREALLOC		(128 * 1024 - METADATA_SIZE - 8)
#define MMAP_THRESHOLD		(128 * 1024)
#define ALIGNMENT 8
#define PADDING(size) (((size) + (ALIGNMENT-1)) & ~(ALIGNMENT-1))

#include "osmem.h"
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include "../utils/block_meta.h"

struct block_meta *start;
struct block_meta *curr;
struct block_meta *last;

struct block_meta *add_block(size_t size, int status, void *location)
{
	curr = (struct block_meta *) location;
	if (curr == NULL)
		return NULL;

	curr->size = size;
	curr->status = status;
	curr->next = NULL;

	if (last == NULL) {
		start = curr;
		last = curr;
	} else {
		last->next = curr;
		curr->prev = last;
		last = curr;
	}

	return curr;
}

void remove_block(void *location)
{
	if (start == location) {
		start = start->next;
		return;
	}

	if (last == location) {
		last = last->prev;
		return;
	}

	struct block_meta *temp = (struct block_meta *) location;

	temp->next->prev = temp->prev;
	temp->prev->next = temp->next;
	temp = NULL;
}

int expand_block(size_t size)
{
	if (start == NULL)
		return 0;

	if (last->status == STATUS_FREE && last->size <= size) {
		sbrk(size - last->size);
		last->status = STATUS_ALLOC;
		last->size = size;
		return 1;
	}
	return 0;
}

void split_block(size_t size, struct block_meta *loc)
{
	struct block_meta *p = ((void *) loc + size + METADATA_SIZE);

	p->prev = loc;
	if (loc->next != NULL) {
		loc->next->prev = p;
		p->next = loc->next;
	} else {
		p->next = NULL;
	}
	loc->next = p;
	int len = loc->size - size - METADATA_SIZE;

	p->size = len;
	loc->size = size;
	p->status = STATUS_FREE;

	if (last == loc)
		last = p;
}

void coalesce(void)
{
	if (start == NULL)
		return;

	struct block_meta *temp = start->next;

	while (temp != NULL) {
		if (temp->status == STATUS_FREE && temp->prev->status == STATUS_FREE) {
			temp->prev->size += temp->size;
			if (temp->next != NULL)
				temp->next->prev = temp->prev;
			temp->prev->next = temp->next;
		}

		temp = temp->next;
	}
}

void *os_malloc(size_t size)
{
	/* TODO: Implement os_malloc */
	if (size == 0)
		return NULL;

	size = PADDING(size);

	coalesce();

	struct block_meta *temp = start;

	// Se verifica blockurile libere
	while (temp != NULL) {
		if (temp->status == STATUS_FREE) {
			if (temp->size >= size) {
				int remaining_size = temp->size - size - METADATA_SIZE;

				if (remaining_size >= 8)
					split_block(size, temp);

				temp->status = STATUS_ALLOC;

				return ((void *) temp+METADATA_SIZE);
			}
		}
		temp = temp->next;
	}

	if (expand_block(size))
		return ((void *)last+METADATA_SIZE);

	struct block_meta *ret;

	// Se creaza un block nou
	if (start == NULL) {
		if (size < MMAP_THRESHOLD) {
			ret = add_block(size, STATUS_ALLOC, sbrk(MMAP_THRESHOLD));
			if (MMAP_THRESHOLD - size - METADATA_SIZE >= 40) {
				add_block(MMAP_THRESHOLD - size - METADATA_SIZE, STATUS_ALLOC,
				 ((void *) ret + size + METADATA_SIZE));
			}
		} else {
			ret = add_block(size, STATUS_MAPPED,
			 mmap(NULL, size + METADATA_SIZE, PROT_READ | PROT_WRITE,
			  MAP_PRIVATE | MAP_ANON, -1, 0));
		}
	} else if (size >= MMAP_THRESHOLD) {
		ret = add_block(size, STATUS_MAPPED,
		 mmap(NULL, size + METADATA_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0));
	} else {
		ret = add_block(size, STATUS_ALLOC, sbrk(size + METADATA_SIZE));
	}

	return ((void *) ret + METADATA_SIZE);
}

void os_free(void *ptr)
{
	/* TODO: Implement os_free */
	if (ptr == NULL)
		return;
	struct block_meta *block = (struct block_meta *) (ptr - METADATA_SIZE);

	if (block->status == STATUS_MAPPED)
		munmap((void *) block, block->size + METADATA_SIZE);
	else
		block->status = STATUS_FREE;
}

void *os_calloc(size_t nmemb, size_t size)
{
	/* TODO: Implement os_calloc */
	if (size == 0 || nmemb == 0)
		return NULL;

	size = PADDING(size*nmemb);

	coalesce();

	struct block_meta *temp = start;

	// Se verifica blockurile libere
	while (temp != NULL) {
		if (temp->status == STATUS_FREE) {
			if (temp->size >= size) {
				int remaining_size = temp->size - size - METADATA_SIZE;

				if (remaining_size >= 40)
					split_block(size, temp);

				temp->status = STATUS_ALLOC;
				temp->size = size;

				return memset(((void *) temp + METADATA_SIZE), 0, temp->size);
			}
		}
		temp = temp->next;
	}

	if (expand_block(size))
		return memset(((void *) last + METADATA_SIZE), 0, last->size);

	struct block_meta *ret;

	// Se creaza un block nou
	if (start == NULL) {
		if ((size_t) getpagesize() > size + METADATA_SIZE) {
			ret = add_block(size, STATUS_ALLOC, sbrk(MMAP_THRESHOLD));
			if (MMAP_THRESHOLD - size - METADATA_SIZE >= 40) {
				add_block(MMAP_THRESHOLD - size - METADATA_SIZE,
				 STATUS_ALLOC, ((void *) ret + size + METADATA_SIZE));
			}
		} else {
			ret = add_block(size, STATUS_MAPPED,
			 mmap(NULL, size + METADATA_SIZE, PROT_READ | PROT_WRITE,
			  MAP_PRIVATE | MAP_ANON, -1, 0));
		}
	} else if ((size_t) getpagesize() <= size + METADATA_SIZE) {
		ret = add_block(size, STATUS_MAPPED,
		 mmap(NULL, size + METADATA_SIZE, PROT_READ | PROT_WRITE,
		  MAP_PRIVATE | MAP_ANON, -1, 0));
	} else {
		ret = add_block(size, STATUS_ALLOC, sbrk(size + METADATA_SIZE));
	}

	return memset(((void *) ret + METADATA_SIZE), 0, ret->size);
}
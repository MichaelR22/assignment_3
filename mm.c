/**
 * @file   mm.c
 * @Author 02335 team
 * @date   September, 2024
 * @brief  Memory management skeleton.
 * 
 */

#include <stdint.h>

#include "mm.h"



/* Proposed data structure elements */

typedef struct header {
  struct header * next;     // Bit 0 is used to indicate free block 
  uint64_t user_block[0];   // Standard trick: Empty array to make sure start of user block is aligned
} BlockHeader;

/* Macros to handle the free flag at bit 0 of the next pointer of header pointed at by p */
#define GET_NEXT(p)    (BlockHeader *) ( (uintptr_t) (p->next) & ~(uintptr_t)1 )    /* Mask out free flag */
#define SET_NEXT(p,n)  do { \
                         uintptr_t flag = (uintptr_t)(p->next) & (uintptr_t)1; \
                         uintptr_t ptr  = ((uintptr_t)(n)) & ~(uintptr_t)1; \
                         p->next = (BlockHeader *)(ptr | flag); \
                       } while(0)  /* Preserve free flag. Uses bitwise NOT + AND to clear the lowest 3 bit */
#define GET_FREE(p)    (uint8_t) ( (((uintptr_t)((p)->next)) & (uintptr_t)0x1) )
#define SET_FREE(p,f)  do { \
                         if (f) p->next = (BlockHeader *)( ((uintptr_t)GET_NEXT(p)) | (uintptr_t)1 ); \
                         else  p->next = GET_NEXT(p); \
                       } while(0) /* Set free bit of p->next to f */
#define SIZE(p)        (size_t) ( (uintptr_t)GET_NEXT(p) - ( (uintptr_t)(p) + sizeof(BlockHeader) ) ) /* Caluculates size of block from p and p->next */

#define MIN_SIZE     (8)   // A block should have at least 8 bytes available for the user


static BlockHeader * first = NULL;
static BlockHeader * current = NULL;

/**
 * @name    simple_init
 * @brief   Initialize the block structure within the available memory
 *
 */
void simple_init() {
  /* Aligns memory start and end to 8 bytes */
  uintptr_t aligned_memory_start = (memory_start + 7) & ~(uintptr_t)7;
  uintptr_t aligned_memory_end   = (memory_end) & ~(uintptr_t)7;
  BlockHeader * last;

  /* Already initalized ? */
  if (first == NULL) {
    /* Check that we have room for at least one free block and an end header */
    if (aligned_memory_start + 2*sizeof(BlockHeader) + MIN_SIZE <= aligned_memory_end) {
      /* Place first and last blocks and set links and free flags properly */
      first = (BlockHeader *) aligned_memory_start;
      last  = (BlockHeader *) (aligned_memory_end - sizeof(BlockHeader));

      /* first free block points to dummy last */
      first->next = last;   /* store raw pointer first */
      SET_FREE(first, 1);   /* mark first as free */

      /* dummy end block: size 0, always allocated, next -> first */
      last->next = first;
      SET_FREE(last, 0);    /* dummy is allocated */
    }
    current = first;     
  } 
}


/**
 * @name    simple_malloc
 * @brief   Allocate at least size contiguous bytes of memory and return a pointer to the first byte.
 *
 * This function should behave similar to a normal malloc implementation. 
 *
 * @param size_t size Number of bytes to allocate.
 * @retval Pointer to the start of the allocated memory or NULL if not possible.
 *
 */
void* simple_malloc(size_t size) {
  
  if (first == NULL) {
    simple_init();
    if (first == NULL) return NULL;
  }

  /* Align requested size to 8 bytes and enforce MIN_SIZE */
  size_t aligned_size = (size + 7) & ~(size_t)7;
  if (aligned_size < MIN_SIZE) {
    aligned_size = MIN_SIZE;
  }

  /* Search for a free block */
  BlockHeader * search_start = current;
  do {
 
    if (GET_FREE(current)) {

      /* Coalesce consecutive free blocks forward to enlarge this free block */
      while (GET_FREE(GET_NEXT(current)) && GET_NEXT(current) != current) {
        BlockHeader * following = GET_NEXT(current);
        /* skip following by linking current to following's next */
        SET_NEXT(current, GET_NEXT(following));
        /* continue while next is also free */
      }

      /* Check if free block is large enough */
      if (SIZE(current) >= aligned_size) {
        /* Will the remainder be large enough for a new block? */
        BlockHeader * ret = current;
        if (SIZE(current) - aligned_size < sizeof(BlockHeader) + MIN_SIZE) {
          /* Use block as is, marking it non-free */
          SET_FREE(current, 0);

          current = GET_NEXT(ret); /* advance current for next-fit */
        } else {
          /* Carve aligned_size from block and allocate new free block for the rest */
          BlockHeader * new_free = (BlockHeader *) ( (uintptr_t)current + sizeof(BlockHeader) + aligned_size );
          /* set new_free->next to the old next and mark new_free free */
          new_free->next = GET_NEXT(current);
          SET_FREE(new_free, 1);

          /* link current to new_free and mark current allocated */
          SET_NEXT(current, new_free);
          SET_FREE(current, 0);

          current = new_free; /* advance current to remainder */

        }
        return (void *) (ret->user_block); /* Return address of current's user_block and advance current */
      }
    }

    current = GET_NEXT(current);
  } while (current != search_start);

 /* None found */
  return NULL;
}


/**
 * @name    simple_free
 * @brief   Frees previously allocated memory and makes it available for subsequent calls to simple_malloc
 *
 * This function should behave similar to a normal free implementation. 
 *
 * @param void *ptr Pointer to the memory to free.
 *
 */
void simple_free(void * ptr) {
  if (ptr == NULL || first == NULL) return;

  /* Find block corresponding to ptr by iterating the ring */
  BlockHeader * head = first;
  BlockHeader * block = NULL;
  do {
    if ((void *) (head->user_block) == ptr) {
      block = head;
      break;
    }
    head = GET_NEXT(head);
  } while (head != first);

  if (block == NULL) {
    /* not found: ignore */
    return;
  }

  if (GET_FREE(block)) {
    /* Block is not in use -- probably an error */
    return;
  }

  /* Mark block free */
  SET_FREE(block, 1);

  /* Coalesce forward while next block is free */
  while (GET_FREE(GET_NEXT(block))) {
    BlockHeader * following = GET_NEXT(block);
    if (following == block) break; /* safety */
    SET_NEXT(block, GET_NEXT(following));
  }

  /* Coalesce backward: find predecessor */
  BlockHeader * prev = first;
  while (GET_NEXT(prev) != block) {
    prev = GET_NEXT(prev);
    /* safety: if we looped full circle without finding block, abort */
    if (prev == first && GET_NEXT(prev) != block) break;
  }
  if (prev != block && GET_FREE(prev)) {
    /* merge prev and block */
    SET_NEXT(prev, GET_NEXT(block));
    block = prev;
  }

  /* Update current to the free block to improve next-fit performance */
  current = block;
}


/* Include test routines */

#include "mm_aux.c"

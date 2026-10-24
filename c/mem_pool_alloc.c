#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#define BLOCK_SIZE      32     // size of each block in bytes
#define NUM_BLOCKS      4     // total blocks in the pool
#define NEXT_BLOCK_REG  0


static size_t curr_free_block;

uint8_t pool[NUM_BLOCKS][BLOCK_SIZE];
void  pool_init(void);
void *pool_alloc(void);      // returns pointer to a free block, or NULL if none available. O(1)
void  pool_free(void *ptr);  // returns a block to the pool. O(1)

/*-----------------------------------------------------------------------*/

/* The following variables will be used for testing only*/
#define TESTING  1

#if TESTING == 1
    static uint8_t allocation_table[NUM_BLOCKS];
#endif
/*----------------------------------------------------- */

void pool_init(void) {

    // Can choose to init all bytes in blocks to 0
    curr_free_block = 0;
    for (size_t i=0; i<NUM_BLOCKS; i++) {
        #if TESTING == 1
            allocation_table[i] = 0;
        #endif

        for (size_t j=0; j<BLOCK_SIZE; j++) {
            if (j == 0) {
                // Point the blocks to the next free one in front of it
                pool[i][j] = (i+1 == NUM_BLOCKS) ? NUM_BLOCKS : i+1;  
            }
            else {
                pool[i][j] = 0;
            }
        }
    }
}

void* pool_alloc(void) {

    size_t give_free_block = curr_free_block;

    if (give_free_block == NUM_BLOCKS) return NULL;

    curr_free_block = pool[give_free_block][NEXT_BLOCK_REG]; // just advance the head


    #if TESTING == 1
        if (give_free_block < NUM_BLOCKS) {
            allocation_table[give_free_block] = 1;
            printf("Successfully allocated block: %d\n",give_free_block);
        }
        else{
            printf("No more free blocks");
        }
    #endif

    return pool[give_free_block];
} 

void pool_free(void *ptr) {
    uint8_t *free_ptr = (uint8_t *)ptr;
    size_t current_block_pos = (free_ptr - pool[0]) / sizeof(pool[0]);

    pool[current_block_pos][NEXT_BLOCK_REG] = curr_free_block; // point to old head (or NUM_BLOCKS sentinel)
    curr_free_block = current_block_pos;                      // this block is now the head

    #if TESTING == 1
        allocation_table[current_block_pos] = 0;
        printf("Succesfully freed block: %d\n",current_block_pos);
    #endif
}

void print_allocation_status(const uint8_t* table) {
    for (size_t i=0; i<NUM_BLOCKS; i++) {
        printf("%d ",table[i]);
    }
    printf("\n");
}

int main(void) {
    #if TESTING == 1
        pool_init();

        uint8_t* block_ptr_0 = (uint8_t* )pool_alloc();
        uint8_t* block_ptr_1 = (uint8_t* )pool_alloc();
        uint8_t* block_ptr_2 = (uint8_t* )pool_alloc();
        uint8_t* block_ptr_3 = (uint8_t* )pool_alloc();
        uint8_t* block_ptr_4 = (uint8_t* )pool_alloc();

        pool_free((void*)block_ptr_2);

        // printf("%d\n",block_ptr_0[NEXT_BLOCK_REG]);
        // printf("%d\n",block_ptr_1[NEXT_BLOCK_REG]);
        // printf("%d\n",block_ptr_3[NEXT_BLOCK_REG]);

        
        print_allocation_status(allocation_table);

        pool_free((void*)block_ptr_3);
        pool_free((void*)block_ptr_1);
        block_ptr_1 = (uint8_t* )pool_alloc();

        print_allocation_status(allocation_table);

    #endif

    return 0;
}


// void pool_free(void *ptr) {
//     uint8_t* free_ptr = (uint8_t*) ptr;
    
//     // Calculate index of block
//     size_t current_block_pos = ((free_ptr - pool[0]) /  sizeof(pool[0]));

//     if (curr_free_block == NUM_BLOCKS) {
//         // If there are no free blocks
//         free_ptr[NEXT_BLOCK_REG] = NUM_BLOCKS;
//         curr_free_block = current_block_pos;
//     }
//     else {
//         // If curr block is pointing to null (NUM_BLOCKS) then point to curr block and make
//         // curr block point to you
//         if (pool[curr_free_block][NEXT_BLOCK_REG] == NUM_BLOCKS) {

//             pool[curr_free_block][NEXT_BLOCK_REG] = current_block_pos;
//             pool[current_block_pos][NEXT_BLOCK_REG] = curr_free_block;
//         }
//         // If curr block is pointing to another block, point to that block first and make curr
//         // block point to you
//         else {
//             pool[current_block_pos][NEXT_BLOCK_REG] = pool[curr_free_block][NEXT_BLOCK_REG];
//             pool[curr_free_block][NEXT_BLOCK_REG] = current_block_pos;
//         }
//     }
    
//     #if TESTING == 1
//         allocation_table[current_block_pos] = 0;
//         printf("Succesfully freed block: %d\n",current_block_pos);
//     #endif

// }
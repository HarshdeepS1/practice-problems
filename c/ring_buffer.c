#define BUF_SIZE 64  // must be a size you choose - think about why


typedef struct {
    uint8_t buffer[BUF_SIZE];
    volatile uint32_t head; // Used for pushing data
    volatile uint32_t tail; // Used for poppping data
} ring_buffer_t;

void rb_init(ring_buffer_t *rb);
bool rb_push(ring_buffer_t *rb, uint8_t byte);   // called from ISR
bool rb_pop(ring_buffer_t *rb, uint8_t *byte);   // called from main loop

// Compile-time check: This catches the problem at compile time with zero runtime cost
_Static_assert(BUF_SIZE > 1, "Ring buffer size must be greater than 1");
/* --------------------------------------------------------------------------------- 
For a ring buffer to be ISR-Safe, meaning an ISR can safely modify it, you would need 
to minimize the chances of having race conditions when reading or writing variables. 
One way is to eliminate the count variable that ordinary ring buffer implementations have
and restrict the ISR to modify only one variable (either the head when pushing data or tail for
popping data). However this comes at a cost of using one space from the buffer.
*/


void rb_init(ring_buffer_t *rb) {

    rb->head = 0;
    rb->tail = 0;

}

bool rb_push(ring_buffer_t *rb, uint8_t byte) {   // called from ISR
    // If head is one in front (wrapped around), then buffer is full
    if (((rb->head + 1) % BUF_SIZE) == rb->tail) return false;
    
    // Push data 
    rb->buffer[rb->head] = byte;

    // Move forwards by one
    rb->head = (rb->head + 1) % BUF_SIZE;
    
    return true;
}

bool rb_pop(ring_buffer_t *rb, uint8_t *byte){   // called from main loop
    // Empty buffer
    if (rb->tail == rb->head) return false;

    // Copy data into the buffer
    *byte = rb->buffer[rb->tail];

    // Move forwards by one
    rb->tail = (rb->tail + 1) % BUF_SIZE;

    return true;
}
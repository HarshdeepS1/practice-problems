/*
Set 1: Bitwise & Bit Manipulation

Problem 1. Write a function that extracts bits [start:end] (inclusive) from a uint32_t register value and returns them right-aligned.
    uint32_t extract_bits(uint32_t reg, uint8_t start, uint8_t end);
    // extract_bits(0b101101, 1, 3) should return 0b110 (i.e. 6)

Problem 2. Write a function to set, clear, and toggle a single bit in a uint8_t, given the bit index.

    void set_bit(uint8_t *val, uint8_t bit_idx);
    void clear_bit(uint8_t *val, uint8_t bit_idx);
    void toggle_bit(uint8_t *val, uint8_t bit_idx);

*/

uint32_t extract_bits(uint32_t reg, uint8_t start, uint8_t end) {
    unsigned int bits_to_align = end - start + 1;

    unsigned int mask = 0; 
    for (unsigned int i=0; i<bits_to_align; i++) {
        mask |= 1u << (start + i); 
    }

    return (reg & (mask)) >> start; 

}


void set_bit(uint8_t *val, uint8_t bit_idx) { // worry about endianness? No since only one byte

    if (bit_idx > 7) return; // Dont need to check if below 0 since unsigned 
    *val |= (uint8_t)(1u << bit_idx); 
    
}

void clear_bit(uint8_t *val, uint8_t bit_idx) {
    if (bit_idx > 7) return;
    *val &= (uint8_t)~(1u << bit_idx);    

}
void toggle_bit(uint8_t *val, uint8_t bit_idx) {
    if (bit_idx > 7) return;
    *val ^= (uint8_t)(1u << bit_idx);    

}

// Problem 3 (gotcha check). What does this print, and why? Fix it if it's wrong.

//     int8_t a = -1;
//     uint8_t b = 255;
//     if (a == b) {
//         printf("equal\n");
//     } else {
//         printf("not equal\n");
//     }

// Then it should print not equal. 


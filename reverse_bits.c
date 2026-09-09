// Reverse the bits of a given 32-bit unsigned integer.

uint32_t reverse_bits(uint32_t n) {

    uint32_t ans = 0;
    uint32_t pos = 0;
    for (int i=31; i>-1; i--) {
        ans |= ((n & (1u << i)) >> i) << pos; // always use 1u since the << and >> operations convert to int 
        pos++;
    }

    return ans;
}

uint32_t reverse_bits_faster(uint32_t n) {


    uint32_t left_bits = (n & (0xAAAAAAAA)) >> 1; 
    uint32_t right_bits = (n & (0x55555555)) << 1;

    uint32_t result = left_bits | right_bits; // Swapped adjacent bits

    left_bits = (result & (0xCCCCCCCC)) >> 2;
    right_bits = (result & (0x33333333)) << 2;

    result = left_bits | right_bits; // Swapped pairs 

    left_bits = (result & (0xF0F0F0F0)) >> 4;
    right_bits = (result & (0x0F0F0F0F)) << 4;

    result = left_bits | right_bits; // Swapped half bytes 

    left_bits = (result & (0xFF00FF00)) >> 8;
    right_bits = (result & (0x00FF00FF)) << 8;

    result = left_bits | right_bits; // Swapped bytes

    left_bits = (result & (0xFFFF0000)) >> 16;
    right_bits = (result & (0x0000FFFF)) << 16;

    result = left_bits | right_bits; // Swapped 16 bits

    return result;

}
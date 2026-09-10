// Write code to determine endianness of system. Return 0 if little endian, return 1 if big endian. 
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

int test_endian(void) {

    uint16_t test = 0x0100; // 0b 0000 0001 0000 0000
    return *(uint8_t*)(&test); 

}

int main(void) {

    int val = test_endian();
    printf("%u\r\n",val);
}

// Reads 4 bytes starting at buf[offset] as a big-endian (this has to be specified) uint32_t,
// regardless of what endianness the host CPU is.

// BTW big endian wire format -> network byte order -> MSB bit is sent first
uint32_t read_be32(const uint8_t *buf, size_t offset) {
    // Example 0b 11110000 00000000

    // if little endian then reads as 0000 0000 0000 1111
    // if big endian reads as         1111 0000 0000 0000

    // Don't need to read and write, lets manually construct the bytes in the right order

    // Ok this will return the correct value, however if the user typcasts to pointer and reads,
    // then it will change depending on endianness. 
    return ((uint32_t)buf[offset] << 24 | (uint32_t)buf[offset+1] << 16 | (uint32_t)buf[offset+2] << 8 |
               (uint32_t)buf[offset+3]);

}
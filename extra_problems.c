#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#define STATE  2


/* 
Problem A — targets: operator precedence, signed-shift UB, dead unsigned bounds check

Write a function that builds a 16-bit CAN-style arbitration ID from three fields packed like this:

bits [15:13] = priority   (3 bits, 0-7)
bits [12:8]  = subsystem  (5 bits, 0-31)
bits [7:0]   = index      (8 bits, 0-255)

uint16_t build_arb_id(uint8_t priority, uint8_t subsystem, uint8_t index);

Also write a bounds-check helper used elsewhere in the same file:

bool is_valid_index(uint8_t index); // valid range: 0-255
*/

uint16_t build_arb_id(uint8_t priority, uint8_t subsystem, uint8_t index) { // 1110 -> E
    uint16_t val = (uint16_t)((priority & 0x7u) << 13) | (uint16_t)(subsystem & 0x1Fu << 8) | (uint16_t)(index);  
    return val;
}

bool is_valid_index(uint8_t index) {
    return true;
}

/*
Problem B — targets: integer promotion / signed-unsigned comparison

You're iterating backward over a telemetry buffer to find the most recent non-zero reading:

int find_last_nonzero(const uint16_t *buf, size_t len) {
    for ( fill in loop ) {
        if (buf[i] != 0) return (int)i;
    }
    return -1;
}

Fill in the loop so it correctly walks from len - 1 down to 0 inclusive, and explain out loud why the obvious version of this loop is a classic bug.
*/

int find_last_nonzero(const uint16_t *buf, size_t len) {
    for (size_t i=len-1; i<len; i--) {
        if (buf[i] != 0) return (int)i;
    }
    return -1;
}

// Alternative approach:
int find_last_nonzero(const uint16_t *buf, size_t len) {
    for (size_t i=len; i>0; i--) {
        if (buf[i-1] != 0) return (int)(i-1);
    }
    return -1;
}

// The bug occurs when we go down all the way down to zero and i-- is calculated, i will wrap around to the highest possible value.

/*
Problem C — targets: reading from output params instead of input, off-by-one buffer bounds

A variable-length telemetry frame is laid out as:

[0]        = payload length N (1 byte, N can be 0-8)
[1..N]     = payload bytes
[N+1]      = checksum, XOR of byte[0] through byte[N]

Write:

bool parse_telemetry(const uint8_t *buf, uint8_t *payload_out, uint8_t *len_out);

payload_out is a caller-provided buffer at least 8 bytes long. On success, copy the N payload bytes into it and write N into *len_out. 
Return false on checksum mismatch (payload_out/len_out can be left untouched or written unconditionally — pick one and be ready to justify it).
*/

bool parse_telemetry(const uint8_t *buf, uint8_t *payload_out, uint8_t *len_out) {
    uint8_t buf_size = buf[0];

    uint8_t checksum = 0; // MUST INITIALIZE or else working with garbage data
    for (size_t i=0; i<buf_size+1; i++) {
        checksum ^= buf[i]; 
    }

    if (checksum == buf[buf_size+1]) {
        for (size_t i=1; i<buf_size+1; i++) {
            payload_out[i-1] = buf[i];
        }
        *len_out = buf_size;
        return true;
    }
    else return false;

}

/*
Problem D — targets: struct trailing padding, alignment terminology

struct Reading {
    uint16_t sensor_id;
    uint8_t  flags;
    double   value;
    uint8_t  valid;
};

Walk through sizeof(struct Reading) by hand on a typical target where double requires 8-byte alignment, 
uint16_t requires 2-byte alignment, and uint8_t requires 1-byte. Show every gap, including the end.
*/

// Answer: 2 + 1 + (5 for padding + 8) + 1 + (7 bytes of padding at end) = 24 bytes total



/*
Problem E — targets: qualifier declaration syntax, &= vs &, volatile mechanism

An ADC peripheral is memory-mapped at 0x40012000 with this layout:

Offset 0x00: CR    (32-bit control register, read/write)
Offset 0x04: SR    (32-bit status register, read-only; bit 1 = "conversion complete")
Offset 0x08: DR    (32-bit data register, read-only)

Write the struct + pointer declaration, a start_conversion(void) function that sets bit 0 of CR, 
and a read_result(void) function that busy-waits until the conversion-complete bit is set in SR, then returns the value in DR.
*/

typedef struct {
    volatile uint32_t CR;
    volatile const uint32_t SR;
    volatile const uint32_t DR;
} ADC;

ADC* adc_ptr = (ADC*) 0x40012000; 

void start_conversion(void) {
    adc_ptr->CR |= (1u << 0);
}

uint32_t read_result(void) {
    while (((adc_ptr->SR & (1u << 1)) >> 1) != 1);

    return adc_ptr->DR;
}


// int main(void) {
//     switch (STATE) {
//         case 1:
//             printf("1");
//             break;

//         case 2:
//             printf("2");
//             break;

//         default:
//             printf("Default");
//     }
// }
/*
Set 2: Byte Arrays, Endianness, Pointer Arithmetic

Problem 4. Write a function that packs a uint32_t into a 4-byte array in big-endian order, and one that unpacks it back — without using htonl/ntohl.


void pack_be32(uint32_t val, uint8_t *buf);
uint32_t unpack_be32(const uint8_t *buf);

Then do the same for little-endian.


Problem 6 (pointer arithmetic). Given uint8_t arr[10] and a uint8_t *p = arr + 3, what does *(p + 2) refer to? 
What about *((uint32_t*)p) — what could go wrong here on some architectures?

Answer: *(p + 2) will give 6th element indexed at 5 of arr. Doing *((uint32_t*)p) will read the next four bytes from arr (arr[3] to arr[6]). 
        Mem addr at p + 3 may not be divisible by type size, causing unaligned access error. Some archs dont support reading unaligned data and will crash.

*/

void pack_be32(uint32_t val, uint8_t *buf) {
    int shift_length = 24;
    for (int i=0; i<4; i++) {
        uint32_t mask = 0xFFu << shift_length; // ALWAYS USE u AS THE LITERAL WILL BECOME INT FIRST SO IF NO SIGN PUTTING 1 IN SIGN BIT (BIT 31) IS UNDEFINED BEHAVIOR
        buf[i] = (val & mask) >> shift_length;
        shift_length -= 8; 
    }
}

uint32_t unpack_be32(const uint8_t *buf) { 
    uint32_t val = (uint32_t)buf[0] << 24 | (uint32_t)buf[1] << 16 | (uint32_t)buf[2] << 8 | (uint32_t)buf[3];
    return val;
    
}

void pack_le32(uint32_t val, uint8_t *buf) {
    int shift_length = 0;
    for (int i=0; i<4; i++) {
        uint32_t mask = 0xFFu << shift_length; // Get the msb first
        buf[i] = (val & mask) >> shift_length;
        shift_length += 8; 
    }
}

uint32_t unpack_le32(const uint8_t *buf) { 
    uint32_t val = (uint32_t)buf[3] << 24 | (uint32_t)buf[2] << 16 | (uint32_t)buf[1] << 8 | (uint32_t)buf[0];
    return val;
    
}

// Problem 5. Given a raw byte buffer representing a sensor frame:

//     [0]     = sensor ID (uint8_t)
//     [1..2]  = sensor value (uint16_t, big-endian)
//     [3]     = checksum (uint8_t, = XOR of bytes 0..2)

// Write a function bool parse_frame(const uint8_t *buf, uint8_t *id, uint16_t *value) that parses it and validates the checksum, returning false on mismatch.

// DIDN'T ASK FOR CLARIFICATION -> the id and value are what the USER WANTS TO FILL OUT. ALWAYS AKS CLARIFYING QUESTIONS

bool parse_frame(const uint8_t *buf, uint8_t *id, uint16_t *value) {
    uint16_t sensor_val = (uint16_t)(buf[1] << 8) | (uint16_t)(buf[2]);
    
    *value = sensor_val;
    *id = buf[0];

    if ((buf[0] ^ buf[1] ^ buf[2]) == (buf[3])) {
        return true;
    }
    else return false;

}


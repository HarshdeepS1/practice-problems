/*Set 3: Struct Packing

Problem 7. Given:


struct Frame {
    uint8_t  id;
    uint32_t timestamp;
    uint16_t value;
};

What is sizeof(struct Frame) likely to be on a typical 32-bit ARM target, and why (walk through the padding)? How would __attribute__((packed)) change it, 
and what's the tradeoff of using it on a struct you memcpy over CAN vs. one you access directly in RAM?
*/

/*Answer: Assuming id starts at some address which is divisible by 4, 4 (3 bytes padding + 1 for id) + 4 (for timestamp) + 2 (for value; current addr is divisible by 2) =  10 bytes.

__attribute__((packed)) would remove the padding so 10 - 3 = 7 bytes. 

__attribute__((packed)) is used for sending over CAN as CAN follows a specific protocol so each byte can be interpreted as something meaningful. 
However if reading from RAM this may cause the CPU to read unaligned data resulting in more cpu cycles. The tradeoff is speed in reading from RAM vs passing unpadded data over network that doesn't expect them.

*/



/*
Problem 8 (replacement) — Register Overlay via Struct

Say you're mapping a peripheral's control registers directly onto memory. The datasheet says:

Base address: 0x40001000
Offset 0x00: CTRL   (32-bit, control register)
Offset 0x04: STATUS (32-bit, read-only status register)
Offset 0x08: DATA   (32-bit, data register)

Part A. Define a struct that overlays this memory layout, and a pointer that lets you access it like regs->CTRL = .... Think carefully about which qualifiers you need and why.

Part B. Write a function void enable_peripheral(void) that sets bit 0 of CTRL to enable the peripheral, using your struct.

Part C. Write a function bool is_peripheral_ready(void) that checks bit 3 of STATUS and returns whether it's set.

Part D (the actual point of this problem). Here's a buggy version someone wrote — find what's wrong and why it could cause a real, hard-to-reproduce hardware bug:

                typedef struct {
                    uint32_t CTRL;
                    uint32_t STATUS;
                    uint32_t DATA;
                } periph_regs_t;

                periph_regs_t *regs = (periph_regs_t *)0x40001000;

                bool wait_for_ready(void) {
                    while (regs->STATUS == 0) {
                        // spin
                    }
                    return true;
                }

Answer: The variables are not volatile. This means compiler will optimize code -> assumes data cannot change externally. So reg->STATUS has value that 
doesn't change inside the while loop -> throw away code. 
*/

// I am mapping this struct over the memory address (make sure the fields align with reg mem addrs)
typedef struct {
    volatile uint32_t CTRL;
    volatile const uint32_t STATUS;
    volatile uint32_t DATA;
    
} periph_regs_t;

periph_regs_t* regs = (periph_regs_t*)0x40001000;

void enable_peripheral(void) {
    regs->CTRL |= 0x00000001;
}


bool is_peripheral_ready(void) {

    // return (regs->STATUS &= (1u << 3)) >> 3; // This assigns it, NOT ALLOWED
    return (regs->STATUS & (1u << 3)) >> 3;

}


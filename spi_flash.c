/*
You're writing firmware to program an external SPI NOR flash chip. Per a (simplified) datasheet excerpt:

Status Register (read via 0x05 command):

Bit 0: WIP (Write In Progress) — 1 while a write/erase operation is ongoing, 0 when idle.
Bit 1: WEL (Write Enable Latch) — must be 1 before a write/erase command will be accepted.
Bits 2-7: reserved / other flags, not relevant here.

You're given these two low-level primitives (already implemented, don't worry about SPI timing):

void spi_send_byte(uint8_t byte);
uint8_t spi_read_byte(void);
void spi_cs_low(void);   // assert chip select
void spi_cs_high(void);  // deassert chip select

uint32_t get_tick_ms(void);

Task: Write a function that reads the status register and returns true once the flash chip is done with its current write/erase operation 
(i.e., blocks/polls until WIP clears), with a maximum timeout of timeout_ms milliseconds — return false if it times out. 
Assume you have uint32_t get_tick_ms(void) available for timing.

bool wait_for_flash_ready(uint32_t timeout_ms);

*/

bool wait_for_flash_ready(uint32_t timeout_ms) {
    spi_cs_low();

    bool flag = false;
    uint8_t byte = 0;
    
    uint32_t time = get_tick_ms();

    spi_send_byte(0x05);

    while ((get_tick_ms() - time) < timeout_ms) {
        byte = spi_read_byte();

        if ((byte & 0x01) == 0) {
            flag = true;
            break;
        }
    }

    spi_cs_high();
    return flag;
}



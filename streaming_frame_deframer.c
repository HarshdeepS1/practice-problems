/*
Bytes arrive one at a time from a UART receive interrupt — you don't get a complete buffer, you get called once per byte. The wire protocol:

0x7E = frame delimiter, marks both the start and end of a frame.

0x7D = escape byte. If a payload byte's real value is 0x7E or 0x7D, the sender transmits 0x7D followed by (real_byte XOR 0x20) instead,
so the delimiter byte never accidentally appears inside a frame's contents.

A frame is just: 0x7E, then zero or more (possibly escaped) payload bytes, then 0x7E.
*/

#define FRAME_DELIM   0x7E
#define FRAME_ESC     0x7D
#define FRAME_ESC_XOR 0x20
#define MAX_FRAME_LEN 32

typedef enum { WAIT_START, IN_FRAME, IN_ESCAPE } deframer_state_t;

typedef struct {
    deframer_state_t state;
    uint8_t buf[MAX_FRAME_LEN];
    uint8_t len;
    bool    frame_ready;   // caller checks this after each call, clears it after consuming buf
} deframer_t;

// Call once per received byte. On completing a valid frame, sets
// d->frame_ready = true and leaves the decoded (unescaped) payload
// in d->buf[0..d->len-1].

// If frame is overflowing then either discard entire frame or have a flag that gets set if overflow happens.
void deframer_process_byte(deframer_t *d, uint8_t byte) {
    if (d->state == WAIT_START) {
        if (byte == FRAME_DELIM ) {
            d->frame_ready = false;
            d->len = 0;
            d->state = IN_FRAME;
        }
        return;
    }

    if (d->state == IN_FRAME) {
        if (byte == FRAME_DELIM) { // Full valid frame
            d->frame_ready = true;
            d->state = WAIT_START;
            return;
        }

        if (byte == FRAME_ESC) {
            d->state = IN_ESCAPE; 
            return;
        }

        if (d->len < MAX_FRAME_LEN) {
            d->buf[d->len++] = byte;
        }
        else {
            // Handle potential overflow -> discard entire message
            d->state = WAIT_START; 
        }
        return;
    }

    if (d->state == IN_ESCAPE) {
        if (byte == FRAME_DELIM || byte == FRAME_ESC) {  // Malformed stream
            d->state = WAIT_START;
            return;
        }
        if (d->len < MAX_FRAME_LEN) {
            d->buf[d->len++] = (byte ^ FRAME_ESC_XOR);
            d->state = IN_FRAME;
        }
        else {
            d->state = WAIT_START; 
        }
        return;
    }
}


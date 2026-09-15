/*
Brake Pressure & Regen Torque Safety Monitor
---------------------------------------------
CAN frame (4 bytes) from brake pressure sensor:
  [0..1] = brake pressure in PSI, uint16_t, big-endian
  [2]    = pedal position, 0-100 (percent)
  [3]    = checksum, XOR of bytes 0..2

System behavior:
  - parse_brake_frame() decodes pressure + pedal position, validates checksum.
  - record_pressure() maintains a rolling history buffer (ring buffer) of
    the last HISTORY_LEN pressure readings for diagnostics.
  - average_pressure() computes the average of all readings currently in
    the history buffer.
  - scale_regen_torque() converts pedal position (0-100%) into a torque
    command in the range 0..max_torque, proportionally.
  - lookup_derate_factor() returns a derating multiplier (0-100) from a
    lookup table indexed by a "zone" (0-4), used to reduce torque near
    thermal limits.
  - check_brake_fault() flags a fault if pressure exceeds MAX_SAFE_PSI
    while pedal position is below MIN_EXPECTED_PEDAL (sensor mismatch:
    high pressure without pedal input suggests a stuck/miscalibrated
    sensor), unless a fault is already latched.
*/

#define HISTORY_LEN        8
#define MAX_SAFE_PSI       1200
#define MIN_EXPECTED_PEDAL 5

typedef struct {
    uint16_t buf[HISTORY_LEN];
    uint8_t  next_slot;
    uint8_t  count;   // number of valid entries, caps at HISTORY_LEN
} pressure_history_t;

static const uint8_t derate_table[5] = {100, 90, 70, 40, 0};

bool parse_brake_frame(const uint8_t *frame, uint16_t *pressure_out, uint8_t *pedal_out) {
    uint8_t checksum = frame[0] ^ frame[1] ^ frame[2];
    if (checksum != frame[3]) {
        return false;
    }
    *pressure_out = (frame[0] << 8) | frame[1];
    *pedal_out = frame[2];
    return true;
}

void record_pressure(pressure_history_t *hist, uint16_t pressure) {
    hist->buf[hist->next_slot] = pressure; 
    hist->next_slot = (hist->next_slot + 1) % HISTORY_LEN;
    if (hist->count < HISTORY_LEN) {
        hist->count++;
    }
}

uint16_t average_pressure(const pressure_history_t *hist) {
    uint8_t sum = 0;
    for (uint8_t i = 0; i < hist->count; i++) { 
        sum += hist->buf[i]; // Using uint8_t -> could overflow
    }
    return sum / hist->count; 
}

uint16_t scale_regen_torque(uint8_t pedal_pct, uint16_t max_torque) {
    uint8_t scaled = (pedal_pct * max_torque) / 100; // The value will be cut down to uint8_t -> truncating data. Also should be using double or floats
    return scaled;
}

uint8_t lookup_derate_factor(uint8_t zone) {
    return derate_table[zone + 1]; // Make sure to check bounds 
}

bool check_brake_fault(uint16_t pressure, uint8_t pedal_pct, bool fault_already_latched) {
    if (pressure > MAX_SAFE_PSI & pedal_pct < MIN_EXPECTED_PEDAL) { // Bitwise works byt && is more robust
        if (!fault_already_latched) {
            return true;   
        }
    }
    return false;
}

int main(void) {
    pressure_history_t hist = {0};
    uint8_t frame[4] = {0x04, 0xB0, 0x02, 0xB6}; // pressure=1200 PSI, pedal=2%

    uint16_t pressure;
    uint8_t pedal;
    if (parse_brake_frame(frame, &pressure, &pedal)) {
        record_pressure(&hist, pressure);
    }

    // No default values given to pedal and pressure if checksum check failed

    uint16_t avg = average_pressure(&hist);
    uint16_t torque = scale_regen_torque(pedal, 500);

    // No check on bounds, if for example given value of 4. Garbage data
    uint8_t derate = lookup_derate_factor(2);
    bool fault = check_brake_fault(pressure, pedal, false);

    printf("avg=%u torque=%u derate=%u fault=%d\n", avg, torque, derate, fault);
    return 0;
}

/*
Overcurrent Protection & Command Processor
-------------------------------------------
Monitors motor current through a debounced safety state machine, and
separately processes variable-length diagnostic commands received over
UART.

STATE MACHINE
  States (enum ocp_state_t):  OCP_NORMAL=0, OCP_WARNING=1, OCP_TRIPPED=2
  Events (enum ocp_event_t):  EVT_CURRENT_OK=0, EVT_CURRENT_HIGH=1, EVT_RESET_CMD=2

  transition_table[state][event] gives the next state.
  EVT_RESET_CMD from OCP_TRIPPED returns to OCP_NORMAL; EVT_RESET_CMD is
  a no-op (stays put) from any other state. EVT_CURRENT_HIGH moves
  NORMAL->WARNING->TRIPPED. EVT_CURRENT_OK from WARNING returns to NORMAL,
  and is a no-op elsewhere.

DEBOUNCE
  Raw current samples arrive via check_overcurrent(), called once per
  millisecond tick (now_ms increments every call, wraps at UINT32_MAX
  per the standard embedded tick counter pattern). If current stays
  above OVERCURRENT_THRESHOLD_MA continuously for DEBOUNCE_MS, this
  function returns EVT_CURRENT_HIGH. If current drops back below
  threshold at any point, the debounce timer must fully reset — a
  later overcurrent excursion needs its own full DEBOUNCE_MS before
  tripping.

COMMAND FRAME (variable length, over UART)
  [0]      = payload length L, 0-16 (does not include header byte or checksum byte)
  [1..L]   = payload bytes
  [L+1]    = checksum, XOR of every byte from [0] through [L] inclusive

  One command type carries a 16-bit "last reported current in mA" as
  the first two payload bytes, little-endian.
*/

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#define OVERCURRENT_THRESHOLD_MA  8000
#define DEBOUNCE_MS               50

typedef enum { OCP_NORMAL = 0, OCP_WARNING = 1, OCP_TRIPPED = 2 } ocp_state_t;
typedef enum { EVT_CURRENT_OK = 0, EVT_CURRENT_HIGH = 1, EVT_RESET_CMD = 2 } ocp_event_t;

typedef struct {
    ocp_state_t state;
} ocp_context_t;

static const ocp_state_t transition_table[3][2] = {
    /* OCP_NORMAL  */ { OCP_NORMAL,  OCP_WARNING },
    /* OCP_WARNING */ { OCP_NORMAL,  OCP_TRIPPED },
    /* OCP_TRIPPED */ { OCP_TRIPPED, OCP_TRIPPED }, // There is no way to get to OCP_NORMAL 
};

void update_state(ocp_context_t ctx, ocp_event_t event) { // Need to pass in pointer to struct 
    ctx.state = transition_table[ctx.state][event]; // If EVT_RESET_CMD then this will result in garbage data being returned
}

ocp_event_t check_overcurrent(uint32_t now_ms, uint16_t current_ma) {
    static uint32_t debounce_start_ms; // Static variables declared inside functions get zero-init
    static bool debounce_active;

    if (current_ma >= OVERCURRENT_THRESHOLD_MA) {
        if (!debounce_active) {
            debounce_active = true;
            debounce_start_ms = now_ms;
        }
        uint32_t elapsed = now_ms - debounce_start_ms;
        if (elapsed >= DEBOUNCE_MS) {
            return EVT_CURRENT_HIGH;
        }
    }
    return EVT_CURRENT_OK;
}

bool parse_command(const uint8_t *frame, uint8_t *payload_out) {
    uint8_t len = frame[0];
    uint8_t checksum = 0;
    for (uint8_t i = 0; i < len; i++) { // Should be i < len + 1
        checksum ^= frame[i];
    }
    if (checksum != frame[len + 1]) {
        return false;
    }
    for (uint8_t i = 0; i < len; i++) {
        payload_out[i] = frame[i + 1];
    }
    return true;
}

uint16_t get_reported_current(const uint8_t *payload) { 
    return (payload[0] << 8) | payload[1]; // Needs to return in little endian so payload[1] should be shifted 8 bits not payload[0]
}

uint8_t count_nonzero_bytes(uint8_t *payload) {
    uint8_t count = 0;
    for (uint8_t i = 0; i < sizeof(payload) / sizeof(payload[0]); i++) { // payload returns bytes of pointer 
        if (payload[i] != 0) count++;
    }
    return count;
}

int main(void) {
    ocp_context_t ctx = { .state = OCP_NORMAL };

    for (uint32_t t = 0; t < 60; t++) {
        ocp_event_t evt = check_overcurrent(t, 9000);
        update_state(ctx, evt);
    }
    printf("State after sustained overcurrent: %d\n", ctx.state);

    uint8_t frame[6] = {4, 0x10, 0x27, 0x00, 0x05, 0x33};
    uint8_t payload[16] = {0};
    bool ok = parse_command(frame, payload);
    printf("parse_command ok=%d\n", ok);

    uint16_t reported = get_reported_current(payload);
    printf("reported current = %u mA\n", reported);

    uint8_t nz = count_nonzero_bytes(payload);
    printf("nonzero bytes = %u\n", nz);

    return 0;
}
#include <stdio.h>
#include "pico/stdlib.h"
#include "config.h"

static bool    stable[NUM_DRIVE][NUM_SENSE];
static uint8_t count[NUM_DRIVE][NUM_SENSE];

static void matrix_init(void) {
    for (int s = 0; s < NUM_SENSE; s++) {
        gpio_init(SENSE_PINS[s]);
        gpio_set_dir(SENSE_PINS[s], GPIO_IN);
        gpio_pull_down(SENSE_PINS[s]);
    }
    for (int d = 0; d < NUM_DRIVE; d++) {
        gpio_init(DRIVE_PINS[d]);
        gpio_set_dir(DRIVE_PINS[d], GPIO_IN);   // idle = high impedance
    }
}

static void matrix_scan(void) {
    for (int d = 0; d < NUM_DRIVE; d++) {
        gpio_set_dir(DRIVE_PINS[d], GPIO_OUT);
        gpio_put(DRIVE_PINS[d], 1);
        sleep_us(5);

        for (int s = 0; s < NUM_SENSE; s++) {
            if (KEY_ID[d][s] == 0) continue;     // empty spot
            bool raw = gpio_get(SENSE_PINS[s]);
            if (raw != stable[d][s]) {
                if (++count[d][s] >= DEBOUNCE_TICKS) {
                    stable[d][s] = raw;
                    count[d][s] = 0;
                    printf("SW%d %s\n", KEY_ID[d][s], raw ? "DOWN" : "UP");
                }
            } else {
                count[d][s] = 0;
            }
        }
        gpio_set_dir(DRIVE_PINS[d], GPIO_IN);
    }
}

// ---- Encoder (quadrature + push switch, active low) ----
static const int8_t enc_table[16] = {0,-1,1,0, 1,0,0,-1, -1,0,0,1, 0,1,-1,0};
static uint8_t enc_state;
static int8_t  enc_accum;
static bool    sw_stable = false;
static uint8_t sw_count;

static void encoder_init(void) {
    uint pins[] = {ENC_A_PIN, ENC_B_PIN, ENC_SW_PIN};
    for (int i = 0; i < 3; i++) {
        gpio_init(pins[i]);
        gpio_set_dir(pins[i], GPIO_IN);
        gpio_pull_up(pins[i]);
    }
    enc_state = (gpio_get(ENC_A_PIN) << 1) | gpio_get(ENC_B_PIN);
}

static void encoder_scan(void) {
    enc_state = ((enc_state << 2) | (gpio_get(ENC_A_PIN) << 1) | gpio_get(ENC_B_PIN)) & 0x0F;
    enc_accum += enc_table[enc_state];
    if (enc_accum >= ENC_STEPS_PER_DETENT)  { enc_accum = 0; printf("ENC CW\n"); }
    if (enc_accum <= -ENC_STEPS_PER_DETENT) { enc_accum = 0; printf("ENC CCW\n"); }

    bool pressed = !gpio_get(ENC_SW_PIN);        // active low
    if (pressed != sw_stable) {
        if (++sw_count >= DEBOUNCE_TICKS) {
            sw_stable = pressed;
            sw_count = 0;
            printf("ENC SW %s\n", pressed ? "DOWN" : "UP");
        }
    } else {
        sw_count = 0;
    }
}

int main(void) {
    stdio_init_all();
    matrix_init();
    encoder_init();
    while (true) {
        matrix_scan();
        encoder_scan();
        sleep_ms(1);
    }
}

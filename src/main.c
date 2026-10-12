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
        sleep_us(5);                            // let the line settle

        for (int s = 0; s < NUM_SENSE; s++) {
            bool raw = gpio_get(SENSE_PINS[s]);
            if (raw != stable[d][s]) {
                if (++count[d][s] >= DEBOUNCE_TICKS) {
                    stable[d][s] = raw;
                    count[d][s] = 0;
                    printf("drive %d / sense %d: %s\n", d, s, raw ? "DOWN" : "UP");
                }
            } else {
                count[d][s] = 0;
            }
        }

        gpio_set_dir(DRIVE_PINS[d], GPIO_IN);   // back to high impedance
    }
}

int main(void) {
    stdio_init_all();
    matrix_init();
    while (true) {
        matrix_scan();
        sleep_ms(1);
    }
}

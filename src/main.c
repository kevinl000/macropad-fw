#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "tusb.h"
#include "config.h"
#include "keymap.h"
#include "usb_descriptors.h"

// ---------------- HID output ----------------
static uint8_t kb_mods;
static uint8_t kb_keys[6];
static bool    kb_dirty;

#define CQ_SIZE 16
static uint16_t cq[CQ_SIZE];
static uint8_t  cq_head, cq_tail;

static void consumer_push(uint16_t usage) {
    uint8_t next = (cq_head + 1) % CQ_SIZE;
    if (next == cq_tail) return;            // queue full, drop
    cq[cq_head] = usage;
    cq_head = next;
}
static void consumer_tap(uint16_t usage) {
    consumer_push(usage);
    consumer_push(0);                       // release
}

static void key_down(uint8_t mods, uint8_t key) {
    kb_mods |= mods;
    for (int i = 0; i < 6; i++) if (kb_keys[i] == key) return;
    for (int i = 0; i < 6; i++) if (kb_keys[i] == 0) { kb_keys[i] = key; break; }
    kb_dirty = true;
}
static void key_up(uint8_t mods, uint8_t key) {
    (void)mods;
    for (int i = 0; i < 6; i++) if (kb_keys[i] == key) kb_keys[i] = 0;
    kb_mods = 0;                            // fine until combos share modifiers
    kb_dirty = true;
}

static void hid_task(void) {
    if (!tud_hid_ready()) return;
    if (kb_dirty) {
        tud_hid_keyboard_report(REPORT_ID_KEYBOARD, kb_mods, kb_keys);
        kb_dirty = false;
    } else if (cq_tail != cq_head) {
        uint16_t usage = cq[cq_tail];
        cq_tail = (cq_tail + 1) % CQ_SIZE;
        tud_hid_report(REPORT_ID_CONSUMER, &usage, sizeof(usage));
    }
}

// (unused for now)
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id,
                               hid_report_type_t report_type,
                               uint8_t *buffer, uint16_t reqlen) {
    (void)instance; (void)report_id; (void)report_type; (void)buffer; (void)reqlen;
    return 0;
}
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id,
                           hid_report_type_t report_type,
                           uint8_t const *buffer, uint16_t bufsize) {
    (void)instance; (void)report_id; (void)report_type; (void)buffer; (void)bufsize;
}

// ---------------- Events ----------------
static void on_key(uint8_t sw, bool down) {
    printf("SW%d %s\n", sw, down ? "DOWN" : "UP");
    const key_action_t *a = &KEYMAP[sw];
    if (down) key_down(a->mods, a->key);
    else      key_up(a->mods, a->key);
}
static void on_encoder_turn(int dir) {
    consumer_tap(dir > 0 ? USAGE_VOL_UP : USAGE_VOL_DOWN);
}
static void on_encoder_press(bool down) {
    if (down) consumer_tap(USAGE_MUTE);
}

// ---------------- Matrix ----------------
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
        gpio_set_dir(DRIVE_PINS[d], GPIO_IN);
    }
}

static void matrix_scan(void) {
    for (int d = 0; d < NUM_DRIVE; d++) {
        gpio_set_dir(DRIVE_PINS[d], GPIO_OUT);
        gpio_put(DRIVE_PINS[d], 1);
        sleep_us(5);

        for (int s = 0; s < NUM_SENSE; s++) {
            if (KEY_ID[d][s] == 0) continue;
            bool raw = gpio_get(SENSE_PINS[s]);
            if (raw != stable[d][s]) {
                if (++count[d][s] >= DEBOUNCE_TICKS) {
                    stable[d][s] = raw;
                    count[d][s] = 0;
                    on_key(KEY_ID[d][s], raw);
                }
            } else {
                count[d][s] = 0;
            }
        }
        gpio_set_dir(DRIVE_PINS[d], GPIO_IN);
    }
}

// ---------------- Encoder ----------------
static const int8_t enc_table[16] = {0,-1,1,0, 1,0,0,-1, -1,0,0,1, 0,1,-1,0};
static uint8_t enc_state;
static int8_t  enc_accum;
static bool    sw_stable;
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
    if (enc_accum >= ENC_STEPS_PER_DETENT)  { enc_accum = 0; on_encoder_turn(+1); }
    if (enc_accum <= -ENC_STEPS_PER_DETENT) { enc_accum = 0; on_encoder_turn(-1); }

    bool pressed = !gpio_get(ENC_SW_PIN);
    if (pressed != sw_stable) {
        if (++sw_count >= DEBOUNCE_TICKS) {
            sw_stable = pressed;
            sw_count = 0;
            on_encoder_press(pressed);
        }
    } else {
        sw_count = 0;
    }
}

int main(void) {
    stdio_init_all();
    tusb_init();
    matrix_init();
    encoder_init();

    while (true) {
        tud_task();
        matrix_scan();
        encoder_scan();
        hid_task();
        sleep_ms(1);
    }
}

#pragma once
#include <stdint.h>
#include "tusb.h"

typedef struct { uint8_t mods; uint8_t key; } key_action_t;

// Index = switch number (SW1..SW14). Index 0 is unused.
static const key_action_t KEYMAP[15] = {
    {0, 0},
    {0, HID_KEY_A}, {0, HID_KEY_B}, {0, HID_KEY_C}, {0, HID_KEY_D},
    {0, HID_KEY_E}, {0, HID_KEY_F}, {0, HID_KEY_G}, {0, HID_KEY_H},
    {0, HID_KEY_I}, {0, HID_KEY_J}, {0, HID_KEY_K}, {0, HID_KEY_L},
    {0, HID_KEY_M}, {0, HID_KEY_N},
};

#define USAGE_VOL_UP    0x00E9
#define USAGE_VOL_DOWN  0x00EA
#define USAGE_MUTE      0x00E2

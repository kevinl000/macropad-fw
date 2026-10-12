#pragma once
#include <stdint.h>

#define NUM_DRIVE 4   // columns (driven)
#define NUM_SENSE 4   // rows (read)

static const uint8_t DRIVE_PINS[NUM_DRIVE] = {6, 7, 8, 9};  // COL0-3 = D6-D9
static const uint8_t SENSE_PINS[NUM_SENSE] = {2, 3, 4, 5};  // ROW0-3 = D2-D5

// Switch number at [column][row]; 0 = no key at that crossing.
static const uint8_t KEY_ID[NUM_DRIVE][NUM_SENSE] = {
    { 1,  4,  7, 10},   // COL0
    { 2,  5,  8, 11},   // COL1
    { 3,  6,  9, 12},   // COL2
    { 0, 13, 14,  0},   // COL3
};

#define ENC_A_PIN   26  // A0
#define ENC_B_PIN   27  // A1
#define ENC_SW_PIN  28  // A2
#define ENC_STEPS_PER_DETENT 4   // adjust once i get the real encoder

#define DEBOUNCE_TICKS 5

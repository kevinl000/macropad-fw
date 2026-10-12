#pragma once
#include <stdint.h>

// "Drive" lines are set high one at a time; "sense" lines are read.
// Diode cathode (striped end) toward the ROW  -> drive = columns, sense = rows
// Diode cathode toward the COLUMN             -> drive = rows,    sense = columns
#define NUM_DRIVE 4
#define NUM_SENSE 4

// TODO: replace with the real GPIO numbers from your schematic
static const uint8_t DRIVE_PINS[NUM_DRIVE] = {6, 7, 8, 9};
static const uint8_t SENSE_PINS[NUM_SENSE] = {2, 3, 4, 5};

#define DEBOUNCE_TICKS 5   // consecutive 1 ms scans before a change counts

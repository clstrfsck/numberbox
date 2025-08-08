/**
 * Copyright (c) 2025 Martin Sandiford.
 * 
 * Project Constants
 */

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <cstdint>

namespace constants {
    // Audio configuration
    constexpr uint SILENCE_SAMPLES = 6615;
    constexpr uint OVERLAP_SAMPLES = 4410;

    // GPIO pins
    constexpr uint USER_LED_PIN = 25;
    constexpr uint WAVESHARE_MP28164_MODE_PIN = 23;
    constexpr uint PICO_FIRST_ADC_PIN = 26;
    constexpr uint PICO_VSYS_PIN = 29;
}

#endif // CONSTANTS_H

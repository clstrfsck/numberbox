/**
 * Copyright (c) 2025 Martin Sandiford.
 * 
 * Project Constants
 */

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <cstddef>

namespace constants {
    // Audio configuration
    constexpr size_t SILENCE_MS = 300;
    constexpr size_t OVERLAP_MS = 200;

    // GPIO pins
    constexpr size_t USER_LED_PIN = 25;
    constexpr size_t WAVESHARE_MP28164_MODE_PIN = 23;
    constexpr size_t PICO_FIRST_ADC_PIN = 26;
    constexpr size_t PICO_VSYS_PIN = 29;
}

#endif // CONSTANTS_H

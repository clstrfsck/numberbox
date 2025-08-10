#include "fail.h"
#include "constants.h"

#include "pico/stdlib.h"

#include "hardware/gpio.h"

namespace {
    // For Seeed Xiao 2350, 0 = on
    // For RPi Pico 2 and Waveshare RP2350 Plus, 1 = on
    constexpr int LED_ON_LEVEL = 1;
    constexpr int LED_OFF_LEVEL = LED_ON_LEVEL ^ 1;

    // Flash length / duty cycle and per-group pause
    constexpr uint32_t BLINK_ON_MS = 300;
    constexpr uint32_t BLINK_OFF_MS = 300;
    constexpr uint32_t CYCLE_PAUSE_MS = 900;

    void led(int what) {
        gpio_put(constants::USER_LED_PIN, what);
    }

    void led_cycle() {
        led(LED_ON_LEVEL);
        sleep_ms(BLINK_ON_MS);
        led(LED_OFF_LEVEL);
        sleep_ms(BLINK_OFF_MS);
    }
}

void fail_init() {
    // Initialize the user LED on Seeed Xiao RP2350 / RPi Pico 2 / Waveshare RP2350 Plus
    gpio_init(constants::USER_LED_PIN);
    gpio_set_dir(constants::USER_LED_PIN, GPIO_OUT);

    // Confidence flash
    led_cycle();
}

void fail(fail_t failure) {
    // Fatal error: blink pattern indefinitely
    while (true) {
        for (int i = 0; i < failure; ++i) {
            led_cycle();
        }
        sleep_ms(CYCLE_PAUSE_MS);
    }
}

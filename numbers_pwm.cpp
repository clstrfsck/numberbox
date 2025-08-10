#include "fail.h"
#include "audio.h"
#include "constants.h"
#include "audio_player.h"
#include "number_to_speech.h"

#include <list>

#include "hardware/clocks.h"

#include "pico/stdlib.h"

namespace {
    // // Power monitoring
    // constexpr uint PICO_POWER_SAMPLE_COUNT = 3;
    // constexpr float VOLTAGE_CONVERSION_FACTOR = 3.0f / (1 << 12);

    // float power_voltage() {
    //     // setup adc
    //     adc_gpio_init(PICO_VSYS_PIN);
    //     adc_select_input(PICO_VSYS_PIN - PICO_FIRST_ADC_PIN);
    
    //     adc_fifo_setup(true, false, 0, false, false);
    //     adc_run(true);

    //     // We seem to read low values initially - this seems to fix it
    //     int ignore_count = PICO_POWER_SAMPLE_COUNT;
    //     while (!adc_fifo_is_empty() || ignore_count-- > 0) {
    //         (void)adc_fifo_get_blocking();
    //     }

    //     // read vsys
    //     uint32_t vsys = 0;
    //     for(int i = 0; i < PICO_POWER_SAMPLE_COUNT; i++) {
    //         uint16_t val = adc_fifo_get_blocking();
    //         vsys += val;
    //     }

    //     adc_run(false);
    //     adc_fifo_drain();

    //     vsys /= PICO_POWER_SAMPLE_COUNT;
    //     // Generate voltage
    //     return vsys * 3 * VOLTAGE_CONVERSION_FACTOR;
    // }

    inline void silence_delay(uint32_t silence_ms) {
        if (silence_ms) {
            // Here we put the buck controller into PSM to reduce power consumption.
            // gpio_put(constants::WAVESHARE_MP28164_MODE_PIN, 0);
            absolute_time_t until = make_timeout_time_ms(silence_ms);
            while (get_absolute_time() <= until) {
                __wfi();
            }
            // gpio_put(constants::WAVESHARE_MP28164_MODE_PIN, 1);
        }
    }
}

int main() {
    set_sys_clock_48mhz();
    fail_init();
    // stdio_init_all();
    // adc_init();

    // For Waveshare RP2350 Plus, we use GPIO 23 to set fixed PWM on for MP28164.
    // This prevents PSM mode and reduces voice frequency output ripple.
    // gpio_init(constants::WAVESHARE_MP28164_MODE_PIN);
    // gpio_set_dir(constants::WAVESHARE_MP28164_MODE_PIN, GPIO_OUT);
    // gpio_put(constants::WAVESHARE_MP28164_MODE_PIN, 0);
    // For now, we are just going to let the pulldown do it's job

    audio_player player;

    uint32_t counter = 1234567;
    std::list<audio_player::sample_data> samples_to_play;
    while (true) {
        const auto tokens = number_to_speech(counter);
        counter += 1;
        if (!tokens.empty()) {
            samples_to_play.clear();
            const auto last_token = tokens.size() - 1;
            for (size_t i = 0; i <= last_token; ++i) {
                bool join = i != last_token && tokens[i + 1] != join_and;
                const auto &sample = audio::get_sample_data(tokens[i]);
                if (sample.data) {
                    samples_to_play.emplace_back(sample.data, sample.size, sample.samples_per_block, join);
                }
            }
            player.play_samples(samples_to_play);
        }
        // Low-power sleep for silence interval
        silence_delay(constants::SILENCE_MS);
    }

    return 0;
}

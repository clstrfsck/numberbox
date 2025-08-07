#include <map>
#include <list>
#include <cstdio>

#include "pico/audio.h"
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "pico/audio_pwm.h"
#include "hardware/clocks.h"

#include "and.h"
#include "billion.h"
#include "eight.h"
#include "eighteen.h"
#include "eighty.h"
#include "eleven.h"
#include "fifteen.h"
#include "fifty.h"
#include "five.h"
#include "forty.h"
#include "four.h"
#include "fourteen.h"
#include "hundred.h"
#include "million.h"
#include "nine.h"
#include "nineteen.h"
#include "ninety.h"
#include "one.h"
#include "seven.h"
#include "seventeen.h"
#include "seventy.h"
#include "six.h"
#include "sixteen.h"
#include "sixty.h"
#include "ten.h"
#include "thirteen.h"
#include "thirty.h"
#include "thousand.h"
#include "three.h"
#include "twelve.h"
#include "twenty.h"
#include "two.h"
#include "zero.h"

#include "audio_player.h"
#include "number_to_speech.h"

#ifndef PICO_POWER_SAMPLE_COUNT
#define PICO_POWER_SAMPLE_COUNT 3
#endif

namespace {
    constexpr uint32_t SILENCE_SAMPLES = 6615;

    constexpr uint USER_LED_PIN = 25;

    constexpr uint WAVESHARE_MP28164_MODE_PIN = 23;
    constexpr uint PICO_FIRST_ADC_PIN = 26;
    constexpr uint PICO_VSYS_PIN = 29;

    typedef audio_player::sample_data sample_data;

    const sample_data number_samples[] = {
        [join_and] = { AND_AUDIO_DATA, AND_SAMPLE_SIZE },
        [billion] = { BILLION_AUDIO_DATA, BILLION_SAMPLE_SIZE },
        [eight] = { EIGHT_AUDIO_DATA, EIGHT_SAMPLE_SIZE },
        [eighteen] = { EIGHTEEN_AUDIO_DATA, EIGHTEEN_SAMPLE_SIZE },
        [eighty] = { EIGHTY_AUDIO_DATA, EIGHTY_SAMPLE_SIZE },
        [eleven] = { ELEVEN_AUDIO_DATA, ELEVEN_SAMPLE_SIZE },
        [fifteen] = { FIFTEEN_AUDIO_DATA, FIFTEEN_SAMPLE_SIZE },
        [fifty] = { FIFTY_AUDIO_DATA, FIFTY_SAMPLE_SIZE },
        [five] = { FIVE_AUDIO_DATA, FIVE_SAMPLE_SIZE },
        [forty] = { FORTY_AUDIO_DATA, FORTY_SAMPLE_SIZE },
        [four] = { FOUR_AUDIO_DATA, FOUR_SAMPLE_SIZE },
        [fourteen] = { FOURTEEN_AUDIO_DATA, FOURTEEN_SAMPLE_SIZE },
        [hundred] = { HUNDRED_AUDIO_DATA, HUNDRED_SAMPLE_SIZE },
        [million] = { MILLION_AUDIO_DATA, MILLION_SAMPLE_SIZE },
        [nine] = { NINE_AUDIO_DATA, NINE_SAMPLE_SIZE },
        [nineteen] = { NINETEEN_AUDIO_DATA, NINETEEN_SAMPLE_SIZE },
        [ninety] = { NINETY_AUDIO_DATA, NINETY_SAMPLE_SIZE },
        [one] = { ONE_AUDIO_DATA, ONE_SAMPLE_SIZE },
        [seven] = { SEVEN_AUDIO_DATA, SEVEN_SAMPLE_SIZE },
        [seventeen] = { SEVENTEEN_AUDIO_DATA, SEVENTEEN_SAMPLE_SIZE },
        [seventy] = { SEVENTY_AUDIO_DATA, SEVENTY_SAMPLE_SIZE },
        [six] = { SIX_AUDIO_DATA, SIX_SAMPLE_SIZE },
        [sixteen] = { SIXTEEN_AUDIO_DATA, SIXTEEN_SAMPLE_SIZE },
        [sixty] = { SIXTY_AUDIO_DATA, SIXTY_SAMPLE_SIZE },
        [ten] = { TEN_AUDIO_DATA, TEN_SAMPLE_SIZE },
        [thirteen] = { THIRTEEN_AUDIO_DATA, THIRTEEN_SAMPLE_SIZE },
        [thirty] = { THIRTY_AUDIO_DATA, THIRTY_SAMPLE_SIZE },
        [thousand] = { THOUSAND_AUDIO_DATA, THOUSAND_SAMPLE_SIZE },
        [three] = { THREE_AUDIO_DATA, THREE_SAMPLE_SIZE },
        [twelve] = { TWELVE_AUDIO_DATA, TWELVE_SAMPLE_SIZE },
        [twenty] = { TWENTY_AUDIO_DATA, TWENTY_SAMPLE_SIZE },
        [two] = { TWO_AUDIO_DATA, TWO_SAMPLE_SIZE },
        [zero] = { ZERO_AUDIO_DATA, ZERO_SAMPLE_SIZE }
    };
    constexpr size_t number_samples_size = sizeof(number_samples) / sizeof(number_samples[0]);

    namespace details {
        void inline write_digits(uint32_t number) {
            if (number >= 10) {
                write_digits(number / 10);
            }
            putchar('0' + (number % 10));
        }
    }

    void write_number(uint32_t number) {
        details::write_digits(number);
        putchar('\n');
    }

    float power_voltage() {
        // setup adc
        adc_gpio_init(PICO_VSYS_PIN);
        adc_select_input(PICO_VSYS_PIN - PICO_FIRST_ADC_PIN);
    
        adc_fifo_setup(true, false, 0, false, false);
        adc_run(true);

        // We seem to read low values initially - this seems to fix it
        int ignore_count = PICO_POWER_SAMPLE_COUNT;
        while (!adc_fifo_is_empty() || ignore_count-- > 0) {
            (void)adc_fifo_get_blocking();
        }

        // read vsys
        uint32_t vsys = 0;
        for(int i = 0; i < PICO_POWER_SAMPLE_COUNT; i++) {
            uint16_t val = adc_fifo_get_blocking();
            vsys += val;
        }

        adc_run(false);
        adc_fifo_drain();

        vsys /= PICO_POWER_SAMPLE_COUNT;
        // Generate voltage
        const float conversion_factor = 3.0f / (1 << 12);
        return vsys * 3 * conversion_factor;
    }
}

int main() {
    set_sys_clock_48mhz();
    stdio_init_all();
    adc_init();

    // Initialize and turn off the user LED on Seeed Xiao RP2350 and Waveshare RP2350 Plus
    gpio_init(USER_LED_PIN);
    gpio_set_dir(USER_LED_PIN, GPIO_OUT);
    // For Seeed Xiao 2350, 1 = off, 0 = on
    // For Waveshare RP2350 Plus, 0 = off, 1 = on
    gpio_put(USER_LED_PIN, 0);

    // For Waveshare RP2350 Plus, we use GPIO 23 to set fixed PWM on for MP28164.
    // This prevents PSM mode and reduces voice frequency output ripple.
    gpio_init(WAVESHARE_MP28164_MODE_PIN);
    gpio_set_dir(WAVESHARE_MP28164_MODE_PIN, GPIO_OUT);
    gpio_put(WAVESHARE_MP28164_MODE_PIN, 1);

    audio_player player;

    puts("Audio device initialized, playing sound...");

    uint32_t counter = 1234567;
    std::list<sample_data> samples_to_play;
    while (true) {
        if (counter % 10 == 0) {
            // Every 100 numbers, print the power voltage
            static const std::map<char, number_token> volts_map = {
                {'0', zero},
                {'1', one},
                {'2', two},
                {'3', three},
                {'4', four},
                {'5', five},
                {'6', six},
                {'7', seven},
                {'8', eight},
                {'9', nine}
            };
            char volts[16];
            float voltage = power_voltage();
            snprintf(volts, sizeof(volts), "%.2f", voltage);
            samples_to_play.clear();
            for (const char * p = volts; *p; ++p) {
                if (volts_map.count(*p)) {
                    samples_to_play.push_back(number_samples[volts_map.at(*p)]);
                } else {
                    samples_to_play.push_back(number_samples[join_and]);
                }
            }
            printf("Power voltage: %.2fV (%s)\n", voltage, volts);
            player.play_silence(SILENCE_SAMPLES * 2);
            player.play_samples(samples_to_play);
            player.play_silence(SILENCE_SAMPLES * 2);
        }
        write_number(counter);
        const auto tokens = number_to_speech(counter);
        counter += 1;
        if (!tokens.empty()) {
            samples_to_play.clear();
            const auto last_token = tokens.size() - 1;
            for (size_t i = 0; i <= last_token; ++i) {
                bool join = i != last_token && tokens[i + 1] != join_and;
                auto sample_index = tokens[i];
                if (sample_index >= 0 && sample_index < number_samples_size) {
                    samples_to_play.push_back(number_samples[sample_index].join(join));
                }
            }
            player.play_samples(samples_to_play);
        }
        player.play_silence(SILENCE_SAMPLES);
    }

    return 0;
}
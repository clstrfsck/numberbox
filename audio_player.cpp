/**
 * Copyright (c) 2025 Martin Sandiford.
 */

#include "audio_player.h"

#include "pico/stdlib.h"
#include "pico/audio_pwm.h"

#include <cstdio>
#include <limits>

typedef audio_player::sample_data sample_data;

namespace {
    constexpr uint32_t OVERLAP_SAMPLES = 4410;

    void fail(const char *message) {
        while (true) {
            while (!stdio_usb_connected()) {
                sleep_ms(1000);
            }
            sleep_ms(5000);
            puts(message);
            while (stdio_usb_connected()) {
                sleep_ms(1000);
            }
        }
    }

    audio_buffer_t *safely_take_audio_buffer(audio_buffer_pool_t *producer_pool) {
        audio_buffer_t *buffer = take_audio_buffer(producer_pool, true);
        if (!buffer) {
            fail("Failed to get free audio buffer");
        }
        return buffer;
    }

    int16_t mix(int16_t a, int16_t b) {
        int32_t raw_result = static_cast<int32_t>(a) + static_cast<int32_t>(b);
        if (raw_result > std::numeric_limits<int16_t>::max()) {
            return std::numeric_limits<int16_t>::max();
        } else if (raw_result < std::numeric_limits<int16_t>::min()) {
            return std::numeric_limits<int16_t>::min();
        }
        return static_cast<int16_t>(raw_result);
    }

    template <typename SampleProcessor>
    void process_audio_data(audio_buffer_pool_t *producer_pool, uint32_t size, SampleProcessor processor) {
        uint32_t data_offset = 0;
        while (data_offset < size) {
            audio_buffer_t *buffer = safely_take_audio_buffer(producer_pool);
            int16_t *samples = reinterpret_cast<int16_t *>(buffer->buffer->bytes);
            uint32_t to_add = std::min(buffer->max_sample_count, size - data_offset);
            
            for (uint32_t n = 0; n < to_add; ++n) {
                samples[n] = processor(data_offset + n);
            }
            
            buffer->sample_count = to_add;
            data_offset += to_add;
            give_audio_buffer(producer_pool, buffer);
        }
    }
} // namespace

audio_player::audio_player() {
    const audio_format_t target_format = {
        .sample_freq = AUDIO_SAMPLE_RATE,
        .format = AUDIO_BUFFER_FORMAT,
        .channel_count = 1
    };
    audio_buffer_format_t buffer_format = {
        .format = &target_format,
        .sample_stride = sizeof(int16_t) / sizeof(uint8_t)
    };

    producer_pool = audio_new_producer_pool(&buffer_format, AUDIO_BUFFER_COUNT, AUDIO_BUFFER_SAMPLE_LENGTH);
    if (!producer_pool) {
        fail("Failed to create producer pool");
    }

    audio_pwm_channel_config_t channel_config = default_mono_channel_config;
    channel_config.core.base_pin = AUDIO_PWM_PIN;

    auto output_format = audio_pwm_setup(&target_format, -1, &channel_config);
    if (!output_format) {
        fail("Failed to setup audio PWM");
    }

    audio_pwm_set_correction_mode(fixed_dither);
    audio_pwm_default_connect(producer_pool, false);
    audio_pwm_set_enabled(true);
}

audio_player::~audio_player() {
    audio_pwm_set_enabled(false);
    // FIXME: Not going to bother with other cleanup for now
}

void audio_player::play_samples(std::list<sample_data> &other_samples) {
    while (!other_samples.empty()) {
        const auto first_sample = other_samples.front();
        other_samples.pop_front();
        play_samples(first_sample.data, first_sample.size, first_sample.join_next, other_samples);
    }
}

void audio_player::play_silence(uint32_t samples) {
    process_audio_data(producer_pool, samples, [](uint32_t) { return int16_t(0); });
}

void audio_player::play_samples(const int16_t *data, uint32_t size, bool join_next, std::list<sample_data> &other_samples) {
    if (join_next) {
        // For most adjacent samples, we overlap them to a degree.
        // In this case, we play the part that does not overlap first...
        if (size > OVERLAP_SAMPLES) {
            // Play the first part before the overlap
            const auto to_play = size - OVERLAP_SAMPLES;
            size -= to_play;
            process_audio_data(producer_pool, to_play, [data](uint32_t index) { return data[index]; });
            data += to_play;
        }
        // ...and then we get the next sample...
        sample_data next_sample = other_samples.empty() ? sample_data{ nullptr, 0 } : other_samples.front();
        if (!other_samples.empty()) {
            other_samples.pop_front();
        }
        auto next_data = next_sample.data;
        auto next_size = next_sample.size;
        // ...and play the overlapping parts from current and next sample...
        if (next_data && next_size) {
            const auto overlapping_len = std::min(size, next_size);
            size -= overlapping_len;
            next_size -= overlapping_len;
            process_audio_data(producer_pool, overlapping_len, [data, next_data](uint32_t index) {
                return mix(data[index], next_data[index]);
            });
            data += overlapping_len;
            next_data += overlapping_len;
            if (next_size) {
                // ...and finally, we play the rest of the next sample,
                // giving it a chance to overlap with following samples.
                play_samples(next_data, next_size, next_sample.join_next, other_samples);
            }
        }
        // In the unusual case where we have no next sample, or the next sample was shorter than
        // the overlap we wanted, we just fall through to play the remainder of the current sample.
    }
    process_audio_data(producer_pool, size, [data](uint32_t index) { return data[index]; });
}

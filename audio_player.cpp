/**
 * Copyright (c) 2025 Martin Sandiford.
 */

#include "fail.h"
#include "constants.h"
#include "pcm_decoder.h"
#include "audio_player.h"

#include "pico/audio_pwm.h"

#include <limits>

typedef audio_player::sample_data sample_data;
typedef pcm_decoder decoder;

namespace {
    constexpr size_t OVERLAP_SAMPLES = constants::OVERLAP_MS * AUDIO_SAMPLE_RATE / 1000;

    audio_buffer_t *safely_take_audio_buffer(audio_buffer_pool_t *producer_pool) {
        audio_buffer_t *buffer = take_audio_buffer(producer_pool, true);
        if (!buffer) {
            fail(FAIL_NO_BUFFER);
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
    void process_audio_data(audio_buffer_pool_t *producer_pool, uint32_t size, SampleProcessor source) {
        uint32_t count = 0;
        while (count < size) {
            audio_buffer_t *buffer = safely_take_audio_buffer(producer_pool);
            int16_t *samples = reinterpret_cast<int16_t *>(buffer->buffer->bytes);
            uint32_t to_add = std::min(buffer->max_sample_count, size - count);

            for (uint32_t n = 0; n < to_add; ++n) {
                samples[n] = source();
            }

            buffer->sample_count = to_add;
            count += to_add;
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
        fail(FAIL_NO_PRODUCER_POOL);
    }

    audio_pwm_channel_config_t channel_config = default_mono_channel_config;
    channel_config.core.base_pin = AUDIO_PWM_PIN;

    auto output_format = audio_pwm_setup(&target_format, -1, &channel_config);
    if (!output_format) {
        fail(FAIL_BAD_OUTPUT_FORMAT);
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
        decoder sample(first_sample.data, first_sample.size, first_sample.block_size);
        play_samples(sample, first_sample.join_next, other_samples);
    }
}

template <typename Decoder>
void audio_player::play_samples(Decoder &sample, bool join_next, std::list<sample_data> &other_samples) {
    if (join_next) {
        // For most adjacent samples, we overlap them to a degree.
        // In this case, we play the part that does not overlap first...
        if (sample.size() > OVERLAP_SAMPLES) {
            // Play the first part before the overlap
            const auto to_play = sample.size() - OVERLAP_SAMPLES;
            process_audio_data(producer_pool, to_play, [&sample]() { return sample.next(); });
        }
        // ...and then we get the next sample...
        sample_data next_sample_data = other_samples.empty() ? sample_data{ nullptr, 0, 0 } : other_samples.front();
        if (!other_samples.empty()) {
            other_samples.pop_front();
        }
        decoder next_sample(next_sample_data.data, next_sample_data.size, next_sample_data.block_size);
        // ...and play the overlapping parts from current and next sample...
        if (!next_sample.empty()) {
            const auto overlapping_len = std::min(sample.size(), next_sample.size());
            process_audio_data(producer_pool, overlapping_len, [&sample, &next_sample]() {
                return mix(sample.next(), next_sample.next());
            });
            if (!next_sample.empty()) {
                // ...and finally, we play the rest of the next sample,
                // giving it a chance to overlap with following samples.
                play_samples(next_sample, next_sample_data.join_next, other_samples);
            }
        }
        // In the unusual case where we have no next sample, or the next sample was shorter than
        // the overlap we wanted, we just fall through to play the remainder of the current sample.
    }
    process_audio_data(producer_pool, sample.size(), [&sample]() { return sample.next(); });
}

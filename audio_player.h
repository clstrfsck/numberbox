/**
 * Copyright (c) 2025 Martin Sandiford.
 */

#ifndef AUDIO_PLAYER_H
#define AUDIO_PLAYER_H

#include "pico/audio.h"
#include "adpcm_decoder.h"

#include <list>
#include <cstdint>
#include <functional>

class audio_player {
public:
    struct sample_data {
        sample_data(const uint8_t *data, size_t size, size_t block_size, bool join_next = false)
            : data(data), size(size), block_size(block_size), join_next(join_next) { }
        const uint8_t *data;
        const size_t size;
        const size_t block_size;
        const bool join_next;

        sample_data join(bool join_next) const {
            return sample_data(data, size, block_size, join_next);
        }
    };

public:
    audio_player();
    ~audio_player();

    void play_samples(std::list<sample_data> &samples_to_play);
    void play_silence(uint32_t samples);

private:
    audio_buffer_pool_t *producer_pool = nullptr;

private:
    void play_samples(adpcm_decoder &sample, bool join_next, std::list<sample_data> &other_samples);
};

#endif // AUDIO_PLAYER_H

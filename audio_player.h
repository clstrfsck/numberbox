/**
 * Copyright (c) 2025 Martin Sandiford.
 */

#ifndef AUDIO_PLAYER_H
#define AUDIO_PLAYER_H

#include "pico/audio.h"

#include <list>
#include <cstdint>
#include <functional>

class audio_player {
public:
    struct sample_data {
        sample_data(const int16_t *data, uint32_t size, bool join_next = false)
            : data(data), size(size), join_next(join_next) { }
        const int16_t *data;
        const uint32_t size;
        const bool join_next;

        sample_data join(bool join_next) const {
            return sample_data(data, size, join_next);
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
    void play_samples(const int16_t *data, uint32_t size, bool join_next, std::list<sample_data> &other_samples);
};

#endif // AUDIO_PLAYER_H

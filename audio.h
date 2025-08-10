#ifndef AUDIO_H
#define AUDIO_H

#include "number_to_speech.h"
#include <cstddef>
#include <cstdint>

namespace audio {
    struct sample_data {
        const uint8_t *data;
        size_t size;
        size_t samples_per_block;
    };

    const sample_data &get_sample_data(number_token index);
}

#endif // AUDIO_H

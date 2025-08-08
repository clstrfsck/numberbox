#include "pcm_decoder.h"

#include <cstdio>
#include <limits>
#include <algorithm>

pcm_decoder::pcm_decoder(const uint8_t* data, size_t data_length, size_t)
    : data(data), bytes_remaining(data_length) {
    // Nothing to do here
}

int16_t pcm_decoder::next() {
    if (empty()) {
        return 0;
    }

    // Read sample (16-bit little-endian)
    int16_t sample = static_cast<int16_t>(data[0] | data[1] << 8);
    data += 2;
    bytes_remaining -= 2;

    return sample;
}

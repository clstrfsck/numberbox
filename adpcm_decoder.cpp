#include "adpcm_decoder.h"
#include <algorithm>
#include <limits>
#include <cstdio>

namespace {
    // IMA ADPCM step size table
    const int16_t step_table[89] = {
        7, 8, 9, 10, 11, 12, 13, 14, 16, 17,
        19, 21, 23, 25, 28, 31, 34, 37, 41, 45,
        50, 55, 60, 66, 73, 80, 88, 97, 107, 118,
        130, 143, 157, 173, 190, 209, 230, 253, 279, 307,
        337, 371, 408, 449, 494, 544, 598, 658, 724, 796,
        876, 963, 1060, 1166, 1282, 1411, 1552, 1707, 1878, 2066,
        2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358,
        5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
        15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
    };

    // IMA ADPCM step index adjustment table
    const int8_t index_table[16] = {
        -1, -1, -1, -1, 2, 4, 6, 8,
        -1, -1, -1, -1, 2, 4, 6, 8
    };

    // std::clamp without the anal retentive type checking
    int32_t clamp_(int32_t value, int32_t min, int32_t max) {
        if (value > max) return max;
        if (value < min) return min;
        return value;
    }
}

adpcm_decoder::adpcm_decoder(const uint8_t* adpcm_data, size_t data_length, size_t block_size)
    : data(adpcm_data), bytes_remaining(data_length), block_size(block_size) {
    // Initialize all state to defaults
    predictor = 0;
    step_index = 0;
    step = step_table[step_index];
    partial_byte = 0;
    high_nibble = false;
    decoding_active = false;

    if (!adpcm_data || data_length == 0) {
        decoding_active = false;
        return;
    }

    if (block_size > 0) {
        // Need to read headers on next sample
        block_remaining = 0;
    } else {
        // No block headers, just raw ADPCM data, 2 samples per byte
        block_remaining = data_length << 1;
    }
    update_decoding_active();
}

int16_t adpcm_decoder::read_header() {
    // IMA ADPCM block header is 4 bytes:
    // Bytes 0-1: Initial predictor value (16-bit little-endian signed)
    // Byte 2:    Initial step index (8-bit unsigned)
    // Byte 3:    Reserved (should be 0)

    if (!decoding_active) {
        // Not decoding, nothing to read
        return 0;
    }

    if (bytes_remaining < 4) {
        // Not enough data for a header, so permanently disable decoding
        bytes_remaining = 0;
        decoding_active = high_nibble = false;
        return 0;
    }

    // Read initial predictor (16-bit little-endian)
    predictor = static_cast<int16_t>(data[0] | data[1] << 8);

    // Read initial step index and clamp it
    step_index = clamp_(data[2], 0, 88);
    // Update step size based on step index
    step = step_table[step_index];

    // Advance past the header
    data += 4;
    bytes_remaining -= 4;

    // Reset block and nibble processing state
    // block_remaining tracks samples remaining in this block (including header sample)
    // We subtract 1, because we are going to return the header sample here.
    block_remaining = block_size - 1;
    high_nibble = false;

    // This is the next sample.
    return static_cast<int16_t>(predictor);
}

void adpcm_decoder::update_decoding_active() {
    decoding_active = (bytes_remaining > 0 || high_nibble);
}

int16_t adpcm_decoder::decode_single_sample(uint8_t adpcm_nibble) {
    // Update step index first, forcing in range
    step_index = clamp_(step_index + index_table[adpcm_nibble & 0x0F], 0, 88);

    // Calculate difference using the optimized bit manipulation method
    int32_t diff = step >> 3;
    if (adpcm_nibble & 4) diff += step;
    if (adpcm_nibble & 2) diff += step >> 1;
    if (adpcm_nibble & 1) diff += step >> 2;

    // Get next step value
    step = step_table[step_index];

    // Apply sign
    if (adpcm_nibble & 8) {
        predictor -= diff;
    } else {
        predictor += diff;
    }

    // Clamp to 16-bit range
    predictor = clamp_(predictor, std::numeric_limits<int16_t>::min(), std::numeric_limits<int16_t>::max());

    return static_cast<int16_t>(predictor);
}

int16_t adpcm_decoder::next() {
    if (block_remaining == 0) {
        return read_header();
    }

    if (!decoding_active) {
        return 0;
    }

    // We are going to return a sample.
    block_remaining -= 1;

    uint8_t nibble;

    if (!high_nibble) {
        // Read new byte for low nibble
        partial_byte = *data;
        nibble = partial_byte & 0x0F;
        high_nibble = true;  // Next will be high nibble
    } else {
        // Process high nibble (second sample in byte)
        nibble = (partial_byte >> 4) & 0x0F;
        high_nibble = false;  // Next will be low nibble of next byte

        // Now we've consumed the entire byte, advance to next
        data += 1;
        bytes_remaining -= 1;
    }

    // Update decoding_active state after processing
    update_decoding_active();

    // Decode the nibble to PCM sample
    return decode_single_sample(nibble);
}

size_t adpcm_decoder::size() const {
    if (!decoding_active) {
        return 0;
    }

    size_t remaining = 0;

    if (block_size > 0) {
        // Block-based mode: samples remaining in current block
        remaining += block_remaining;

        // Calculate bytes remaining in current block
        // block_remaining samples need (block_remaining + 1) / 2 bytes
        size_t remaining_block_bytes = (block_remaining + 1) / 2;

        if (remaining_block_bytes < bytes_remaining) {
            // Need to calculate number of full and partial blocks in remaining data
            size_t bytes_after_current_block = bytes_remaining - remaining_block_bytes;
            // Each complete block has: 4 bytes header + (block_size - 1) / 2 bytes data
            // (block_size is odd and includes header sample, so data portion is even)
            size_t bytes_per_block = 4 + block_size / 2;
            size_t complete_blocks = bytes_after_current_block / bytes_per_block;
            remaining += complete_blocks * block_size;

            // Handle any remaining partial block
            size_t leftover_bytes = bytes_after_current_block % bytes_per_block;
            if (leftover_bytes >= 4) {
                // We have at least a header, so we can decode the header sample,
                // plus any additional samples from the data portion
                size_t data_bytes = leftover_bytes - 4;
                remaining += 1 + data_bytes * 2; // Each byte contains 2 samples
            }
        }
    } else {
        // Raw mode: each byte contains 2 samples
        remaining += bytes_remaining * 2;

        // When high_nibble is true, we've already consumed the low nibble
        // of the current byte, but bytes_remaining still counts that byte
        if (remaining > 0 && high_nibble) {
            remaining -= 1;
        }
    }

    return remaining;
}

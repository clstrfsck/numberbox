#ifndef PCM_DECODER_H
#define PCM_DECODER_H

#include <cstddef>
#include <cstdint>

class pcm_decoder {
public:
    /**
     * Constructor
     * 
     * @param pcm_data Pointer to signed LE 16 bit data to decode
     * @param data_length Total length of data in bytes
     * @param block_size Size of each sample block in bytes. Ignored for PCM data.
     */
    pcm_decoder(const uint8_t *pcm_data, size_t data_length, size_t block_size) : data(pcm_data), bytes_remaining(data_length) {
        // Nothing to do here
    }

    /**
     * Decode and return a single sample
     * 
     * Convenient method for sample-by-sample processing.
     * 
     * @return Next 16-bit PCM sample, or 0 if no more data available
     */
    int16_t next() {
        if (empty()) {
            return 0;
        }

        // Read sample (16-bit little-endian)
        int16_t sample = static_cast<int16_t>(data[0] | data[1] << 8);
        data += 2;
        bytes_remaining -= 2;

        return sample;
    }

    /**
     * Check if there is more data to decode
     * 
     * @return true if more data is available, false if decoding is complete
     */
    bool empty() const { return bytes_remaining < 2; }

    /**
     * Get the number of samples remaining to be decoded
     * 
     * @return Number of 16-bit PCM samples that will be produced from remaining data
     */
    size_t size() const { return bytes_remaining / 2; } // Each sample is 2 bytes

private:
    // Input data
    const uint8_t *data;             // Pointer to current position in ADPCM data
    size_t bytes_remaining;          // Remaining bytes to decode
};

#endif // PCM_DECODER_H

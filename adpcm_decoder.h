#ifndef ADPCM_DECODER_H
#define ADPCM_DECODER_H

#include <cstdint>
#include <cstddef>

/**
 * IMA ADPCM Decoder
 *
 * Converts IMA ADPCM compressed audio data to signed 16-bit PCM samples.
 * IMA ADPCM uses 4-bit samples with adaptive quantization.
 */
class adpcm_decoder {
public:
    /**
     * Constructor
     *
     * @param adpcm_data Pointer to ADPCM data to decode
     * @param data_length Total length of data in bytes
     * @param block_size Size of each sample block in bytes.
     *          Use zero for raw ADPCM data without block headers.
     */
    adpcm_decoder(const uint8_t *adpcm_data, size_t data_length, size_t block_size);

    /**
     * Decode and return a single sample
     *
     * Convenient method for sample-by-sample processing.
     *
     * @return Next 16-bit PCM sample, or 0 if no more data available
     */
    int16_t next();

    /**
     * Check if there is more data to decode
     *
     * @return true if more data is available, false if decoding is complete
     */
    bool empty() const { return !decoding_active; }

    /**
     * Get the number of samples remaining to be decoded
     *
     * @return Number of 16-bit PCM samples that will be produced from remaining data
     */
    size_t size() const;

private:
    // Input data
    const uint8_t *data;             // Pointer to current position in ADPCM data
    size_t bytes_remaining;          // Remaining bytes to decode

    // Block header state
    const size_t block_size;         // Size of each sample block in bytes
    size_t block_remaining;          // Bytes remaining in the current block

    // Decoder state
    int32_t predictor;               // Current predicted sample value
    int32_t step_index;              // Current step index for quantization
    int32_t step;                    // Current step size for quantization

    // Partial decoding state
    uint8_t partial_byte;            // Current byte being processed
    bool high_nibble;                // true if next sample is high nibble
    bool decoding_active;            // true if partial decoding is active

    int16_t decode_single_sample(uint8_t adpcm_sample);
    int16_t read_header();
    void update_decoding_active();
};

#endif // ADPCM_DECODER_H

/**
 * Copyright (c) 2025 Martin Sandiford.
 *
 * British English Number to Speech Conversion - Implementation
 */

#include "number_to_speech.h"

#include <array>

namespace {
    constexpr uint32_t BILLION  = 1000000000;
    constexpr uint32_t MILLION  = 1000000;
    constexpr uint32_t THOUSAND = 1000;
    constexpr uint32_t HUNDRED  = 100;
    constexpr uint32_t TWENTY   = 20;
    constexpr uint32_t TEN      = 10;

    number_token ones_to_word(uint32_t ones) {
        static const std::array<number_token, 20> ones_words = {
            error, one, two, three, four, five, six, seven, eight, nine,
            ten, eleven, twelve, thirteen, fourteen, fifteen, sixteen,
            seventeen, eighteen, nineteen
        };

        if (ones < ones_words.size()) {
            return ones_words[ones];
        }
        return error;
    }

    number_token tens_to_word(uint32_t tens) {
        static const std::array<number_token, 10> tens_words = {
            error, error, twenty, thirty, forty, fifty, sixty, seventy, eighty, ninety
        };

        if (tens < tens_words.size()) {
            return tens_words[tens];
        }
        return error;
    }

    void hundreds_to_tokens(std::vector<number_token>& tokens, uint32_t number, bool add_and) {
        if (number == 0) {
            return; // Return early for zero
        }

        // Handle hundreds
        uint32_t hundreds = number / HUNDRED;
        uint32_t remainder = number % HUNDRED;

        if (hundreds > 0) {
            tokens.push_back(ones_to_word(hundreds));
            tokens.push_back(hundred);
            add_and = true;
        }
        if (add_and && remainder > 0) {
            tokens.push_back(join_and);
        }

        // Handle tens and ones
        if (remainder >= TWENTY) {
            uint32_t tens = remainder / TEN;
            uint32_t ones = remainder % TEN;

            tokens.push_back(tens_to_word(tens));
            if (ones > 0) {
                tokens.push_back(ones_to_word(ones));
            }
        } else if (remainder > 0) {
            tokens.push_back(ones_to_word(remainder));
        }
    }
}

std::vector<number_token> number_to_speech(uint32_t number) {
    std::vector<number_token> tokens;

    // Handle zero as special case
    if (number == 0) {
        tokens.push_back(zero);
        return tokens;
    }

    // Handle billions (1,000,000,000 to 4,294,967,295)
    if (number >= BILLION) {
        uint32_t billions = number / BILLION;
        hundreds_to_tokens(tokens, billions, false);
        tokens.push_back(billion);
        number %= BILLION;
    }

    // Handle millions (1,000,000 to 999,999,999)
    if (number >= MILLION) {
        uint32_t millions = number / MILLION;
        hundreds_to_tokens(tokens, millions, false);
        tokens.push_back(million);
        number %= MILLION;
    }

    // Handle thousands (1,000 to 999,999)
    if (number >= THOUSAND) {
        uint32_t thousands = number / THOUSAND;
        hundreds_to_tokens(tokens, thousands, false);
        tokens.push_back(thousand);
        number %= THOUSAND;
    }

    // Handle remaining hundreds, tens, and ones
    if (number > 0) {
        bool add_and = !tokens.empty(); // Add "and" if there are previous tokens (British style)
        hundreds_to_tokens(tokens, number, add_and);
    }

    return tokens;
}

/**
 * Copyright (c) 2025 Martin Sandiford.
 * 
 * British English Number to Speech Conversion
 */

#ifndef NUMBER_TO_SPEECH_H
#define NUMBER_TO_SPEECH_H

#include <vector>
#include <cstdint>

enum number_token {
  error = -1,
  join_and = 0,
  billion,
  eight,
  eighteen,
  eighty,
  eleven,
  fifteen,
  fifty,
  five,
  forty,
  four,
  fourteen,
  hundred,
  million,
  nine,
  nineteen,
  ninety,
  one,
  seven,
  seventeen,
  seventy,
  six,
  sixteen,
  sixty,
  ten,
  thirteen,
  thirty,
  thousand,
  three,
  twelve,
  twenty,
  two,
  zero
};

/**
 * Converts an integer to a vector of British English speech tokens.
 * 
 * @param number The integer to convert (supports all unsigned 32 bit numbers)
 * @return A vector of tokens representing the number in British English
 * 
 * Examples:
 * - 0 -> {zero}
 * - 13 -> {thirteen}
 * - 21 -> {twenty, one}
 * - 100 -> {one, hundred}
 * - 101 -> {one, hundred, join_and, one}
 * - 1234 -> {one, thousand, two, hundred, join_and, thirty, four}
 */
std::vector<number_token> number_to_speech(uint32_t number);

#endif // NUMBER_TO_SPEECH_H

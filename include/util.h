#pragma once

#include <string>

/// Number of letters in the Latin alphabet.
inline constexpr std::size_t k_alphabet_size = 26;

std::string score_feedback(std::string_view guess, std::string_view target);

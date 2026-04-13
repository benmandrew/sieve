#include <array>

#include "util.h"

std::string score_feedback(std::string_view guess, std::string_view target) {
    std::string feedback(guess.size(), 'b');
    std::array<int, k_alphabet_size> remaining{};
    for (std::size_t i = 0; i < guess.size(); ++i) {
        if (guess[i] == target[i]) {
            feedback[i] = 'g';
        } else {
            ++remaining[static_cast<std::size_t>(target[i] - 'a')];
        }
    }
    for (std::size_t i = 0; i < guess.size(); ++i) {
        if (feedback[i] == 'g') {
            continue;
        }
        const std::size_t index = static_cast<std::size_t>(guess[i] - 'a');
        if (remaining[index] > 0) {
            feedback[i] = 'y';
            --remaining[index];
        }
    }
    return feedback;
}
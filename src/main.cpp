#include <string>
#include <fstream>
#include <iostream>
#include <cassert>

#include "bitset_index.h"

#define BUF_SIZE 16
#define STR_LEN 5


// Prints the guess string with colored backgrounds according to feedback string
// feedback: 'g' = green, 'y' = yellow, 'b' = black/grey
void print_colored_guess(const std::string& guess, const std::string& feedback) {
    assert(guess.size() == feedback.size());
    for (size_t i = 0; i < guess.size(); ++i) {
        std::string color_code;
        switch (feedback[i]) {
            case 'g': color_code = "\033[42;30m"; break; // Green bg, black fg
            case 'y': color_code = "\033[43;30m"; break; // Yellow bg, black fg
            case 'b': color_code = "\033[100m"; break; // Grey bg, default fg
            default: color_code = "\033[0m"; break; // Reset
        }
        std::cout << color_code << ' ' << guess[i] << ' ' << "\033[0m";
    }
    std::cout << std::endl;
}

int main(int _argc, char** _argv) {
    auto file = std::ifstream("resources/words.txt");
    std::vector<std::string> words;
    words.reserve(128);
    char buf[BUF_SIZE];
    while (!file.eof()) {
        file.getline(buf, BUF_SIZE);
        if (strlen(buf) == STR_LEN) {
            words.push_back(buf);
        }
    }
    sieve::BitsetIndex index(words);
    auto view = index.all_words();
    const std::vector<std::string> guesses = {
        "rebut", "sissy", "cigar",
    };
    const std::vector<std::string> guess_feedbacks = {
        "ybbbb", "bgbbb", "ggggg",
    };
    for (int i = 0; i < 3; i++) {
        print_colored_guess(guesses[i], guess_feedbacks[i]);
        view.apply_feedback(guesses[i], guess_feedbacks[i]);
    }
    std::size_t idx = 0;
    const std::string *candidate = view.next_candidate(idx);
    while (candidate != nullptr) {
        std::cout << *candidate << "\n";
        candidate = view.next_candidate(idx);
    }
    return 0;
}

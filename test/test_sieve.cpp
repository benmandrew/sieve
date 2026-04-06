#include "bitset_index.h"

#include <array>
#include <cassert>
#include <stdexcept>

namespace {

const std::vector<std::string> k_words = {
    "cigar", "rebut", "sissy", "humph", "awake", "blush", "focal", "evade",
    "naval", "serve", "heath", "dwarf", "model", "karma", "stink", "grade",
    "quiet", "bench", "abate", "feign", "major", "death", "fresh", "crust",
    "stool", "colon", "abase", "marry", "react", "batty"};

std::string score_feedback(std::string_view guess, std::string_view target) {
  std::string feedback(guess.size(), 'b');
  std::array<int, 26> remaining{};
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

void test_index_and_full_view() {
  sieve::BitsetIndex index(k_words);
  auto view = index.all_words();

  assert(index.word_length() == 5);
  assert(index.dictionary_size() == k_words.size());
  assert(view.candidate_count() == k_words.size());
}

void test_green_constraints() {
  sieve::BitsetIndex index(k_words);
  auto view = index.all_words();

  view.apply_feedback("cigar", "gbbbb");
  std::size_t cursor = 0;
  while (const std::string *candidate = view.next_candidate(cursor)) {
    assert((*candidate)[0] == 'c');
  }
}

void test_grey_constraints() {
  sieve::BitsetIndex index(k_words);
  auto view = index.all_words();

  view.apply_feedback("xxxxx", "bbbbb");
  assert(view.candidate_count() == k_words.size());
  std::size_t cursor = 0;
  while (const std::string *candidate = view.next_candidate(cursor)) {
    for (const char c : *candidate) {
      assert(c != 'x');
    }
  }
}

void test_yellow_and_position_exclusion() {
  sieve::BitsetIndex index(k_words);
  auto view = index.all_words();

  view.apply_feedback("awake", "bybbb");
  std::size_t cursor = 0;
  while (const std::string *candidate = view.next_candidate(cursor)) {
    bool has_w = false;
    for (const char c : *candidate) {
      if (c == 'w') {
        has_w = true;
        break;
      }
    }
    assert(has_w);
    assert((*candidate)[1] != 'w');
  }
}

void test_duplicate_letter_feedback() {
  const std::vector<std::string> words = {"array", "carry", "harry", "rarer",
                                          "rally", "cairn", "parry"};
  sieve::BitsetIndex index(words);
  auto view = index.all_words();

  const std::string guess = "rarer";
  const std::string feedback = "yggbb";
  view.apply_feedback(guess, feedback);

  std::vector<std::string> expected;
  for (const std::string &word : words) {
    if (score_feedback(guess, word) == feedback) {
      expected.push_back(word);
    }
  }

  assert(!expected.empty());
  assert(view.candidate_count() == expected.size());

  std::vector<std::string> actual;
  std::size_t cursor = 0;
  while (const std::string *candidate = view.next_candidate(cursor)) {
    actual.push_back(*candidate);
  }

  assert(actual == expected);
}

void test_invalid_input_rejected() {
  bool threw = false;
  try {
    sieve::BitsetIndex bad_words({"Hello"});
    (void)bad_words;
  } catch (const std::invalid_argument &) {
    threw = true;
  }
  assert(threw);

  sieve::BitsetIndex index(k_words);
  auto view = index.all_words();

  threw = false;
  try {
    view.apply_feedback("abcde", "ggxxg");
  } catch (const std::invalid_argument &) {
    threw = true;
  }
  assert(threw);
}

} // namespace

int main() {
  test_index_and_full_view();
  test_green_constraints();
  test_grey_constraints();
  test_yellow_and_position_exclusion();
  test_duplicate_letter_feedback();
  test_invalid_input_rejected();
  return 0;
}

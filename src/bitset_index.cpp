#include "bitset_index.h"

#include <array>
#include <limits>
#include <stdexcept>
#include <utility>

namespace sieve {
/// @cond INTERNAL
namespace {

using Bitset = BitsetIndex::Bitset;

/// Set one bit in a dynamic bitset.
///
/// @param bitset Bitset to modify.
/// @param index Bit index to set.
void set_bit(Bitset &bitset, std::size_t index) {
  const std::size_t word = index / 64;
  const std::size_t bit = index % 64;
  bitset[word] |= (static_cast<std::uint64_t>(1) << bit);
}

/// Test one bit in a dynamic bitset.
///
/// @param bitset Bitset to query.
/// @param index Bit index to test.
/// @return True if the indexed bit is set.
bool test_bit(const Bitset &bitset, std::size_t index) {
  const std::size_t word = index / 64;
  const std::size_t bit = index % 64;
  return ((bitset[word] >> bit) & static_cast<std::uint64_t>(1)) != 0;
}

/// Apply bitwise AND into target.
///
/// @param target Bitset updated in place.
/// @param mask Bitset mask to AND with target.
void bit_and(Bitset &target, const Bitset &mask) {
  for (std::size_t i = 0; i < target.size(); ++i) {
    target[i] &= mask[i];
  }
}

/// Apply bitwise AND-NOT into target.
///
/// @param target Bitset updated in place.
/// @param mask Bitset mask whose set bits are cleared from target.
void bit_and_not(Bitset &target, const Bitset &mask) {
  for (std::size_t i = 0; i < target.size(); ++i) {
    target[i] &= ~mask[i];
  }
}

/// Compute bitwise OR across a subrange of bitset masks.
///
/// @tparam MaskContainer Container type indexed by mask index.
/// @param masks Mask container.
/// @param begin_index Inclusive start index.
/// @param end_index_exclusive Exclusive end index.
/// @param word_count Number of uint64 words per mask.
/// @return Combined OR mask.
template <typename MaskContainer>
Bitset bit_or(const MaskContainer &masks, std::size_t begin_index,
              std::size_t end_index_exclusive, std::size_t word_count) {
  Bitset result(word_count, 0);
  if (begin_index >= end_index_exclusive) {
    return result;
  }
  for (std::size_t i = begin_index; i < end_index_exclusive; ++i) {
    for (std::size_t w = 0; w < word_count; ++w) {
      result[w] |= masks[i][w];
    }
  }
  return result;
}

/// Count set bits across a dynamic bitset.
///
/// @param bitset Input bitset.
/// @return Number of set bits.
std::size_t popcount_bitset(const Bitset &bitset) {
  std::size_t total = 0;
  for (const std::uint64_t word : bitset) {
#if defined(__GNUC__) || defined(__clang__)
    total += static_cast<std::size_t>(__builtin_popcountll(word));
#else
    std::uint64_t x = word;
    while (x != 0) {
      x &= (x - 1);
      ++total;
    }
#endif
  }
  return total;
}

} // namespace
/// @endcond

FilterView::FilterView(const BitsetIndex &index, bool all_candidates)
    : m_index(index),
      m_candidates(all_candidates ? index.full_mask() : index.empty_mask()) {}

void FilterView::reset() { m_candidates = m_index.full_mask(); }

void FilterView::clear() { m_candidates = m_index.empty_mask(); }

void FilterView::apply_feedback(std::string_view guess,
                                std::string_view feedback) {
  if (guess.size() != m_index.m_word_length ||
      feedback.size() != m_index.m_word_length) {
    throw std::invalid_argument(
        "guess and feedback must match dictionary word length");
  }
  letter_count_array min_count{};
  letter_count_array max_count{};
  letter_flag_array has_grey{};
  initialize_letter_constraints(min_count, max_count, has_grey);
  apply_position_constraints(guess, feedback, min_count, has_grey);
  apply_count_constraints(min_count, max_count, has_grey);
}

void FilterView::initialize_letter_constraints(
    letter_count_array &min_count, letter_count_array &max_count,
    letter_flag_array &has_grey) const {
  min_count.fill(0);
  has_grey.fill(false);
  max_count.fill(m_index.m_word_length);
}

void FilterView::apply_position_constraints(std::string_view guess,
                                            std::string_view feedback,
                                            letter_count_array &min_count,
                                            letter_flag_array &has_grey) {
  for (std::size_t pos = 0; pos < m_index.m_word_length; ++pos) {
    const int letter = BitsetIndex::char_to_index(guess[pos]);
    const char mark = feedback[pos];
    if (mark != BitsetIndex::k_green && mark != BitsetIndex::k_yellow &&
        mark != BitsetIndex::k_grey) {
      throw std::invalid_argument("feedback must use only 'g', 'y', or 'b'");
    }
    if (mark == BitsetIndex::k_green || mark == BitsetIndex::k_yellow) {
      ++min_count[static_cast<std::size_t>(letter)];
    }
    if (mark == BitsetIndex::k_grey) {
      has_grey[static_cast<std::size_t>(letter)] = true;
    }
    if (mark == BitsetIndex::k_green) {
      bit_and(
          m_candidates,
          m_index
              .m_position_letter_masks[pos][static_cast<std::size_t>(letter)]);
    } else {
      bit_and_not(
          m_candidates,
          m_index
              .m_position_letter_masks[pos][static_cast<std::size_t>(letter)]);
    }
  }
}

void FilterView::apply_count_constraints(const letter_count_array &min_count,
                                         const letter_count_array &max_count,
                                         const letter_flag_array &has_grey) {
  letter_count_array effective_max_count = max_count;
  for (std::size_t letter = 0; letter < BitsetIndex::k_alphabet_size;
       ++letter) {
    if (has_grey[letter]) {
      effective_max_count[letter] = min_count[letter];
    }
    const std::size_t min_c = min_count[letter];
    const std::size_t max_c = effective_max_count[letter];
    if (min_c > max_c) {
      clear();
      return;
    }
    const Bitset allowed = bit_or(m_index.m_letter_count_masks[letter], min_c,
                                  max_c + 1, m_candidates.size());
    bit_and(m_candidates, allowed);
  }
}

std::size_t FilterView::candidate_count() const {
  return popcount_bitset(m_candidates);
}

bool FilterView::is_candidate(std::size_t index) const {
  if (index >= m_index.m_words.size()) {
    throw std::out_of_range("candidate index is out of range");
  }
  return test_bit(m_candidates, index);
}

const std::string *FilterView::next_candidate(std::size_t &cursor) const {
  while (cursor < m_index.m_words.size()) {
    const std::size_t current = cursor;
    ++cursor;
    if (test_bit(m_candidates, current)) {
      return &m_index.m_words[current];
    }
  }
  return nullptr;
}

BitsetIndex::BitsetIndex(std::vector<std::string> words)
    : m_words(std::move(words)), m_word_length(0), m_bit_count(0),
      m_position_letter_masks(), m_letter_count_masks() {
  validate_words_and_initialize_length();
  build_lookup_masks();
}

void BitsetIndex::validate_words_and_initialize_length() {
  if (m_words.empty()) {
    throw std::invalid_argument("word dictionary cannot be empty");
  }
  m_word_length = m_words.front().size();
  if (m_word_length == 0) {
    throw std::invalid_argument("words must be non-empty");
  }
  for (const std::string &word : m_words) {
    if (word.size() != m_word_length) {
      throw std::invalid_argument("all words must have equal length");
    }
    for (const char c : word) {
      char_to_index(c);
    }
  }
}

void BitsetIndex::build_lookup_masks() {
  m_bit_count = bitset_word_count(m_words.size());
  m_position_letter_masks.assign(m_word_length, {});
  for (auto &letter_masks : m_position_letter_masks) {
    for (Bitset &mask : letter_masks) {
      mask.assign(m_bit_count, 0);
    }
  }
  for (std::vector<Bitset> &count_masks : m_letter_count_masks) {
    count_masks.assign(m_word_length + 1, Bitset(m_bit_count, 0));
  }
  for (std::size_t word_index = 0; word_index < m_words.size(); ++word_index) {
    const std::string &word = m_words[word_index];
    std::array<std::size_t, k_alphabet_size> counts{};
    for (std::size_t pos = 0; pos < m_word_length; ++pos) {
      const int letter = char_to_index(word[pos]);
      set_bit(m_position_letter_masks[pos][static_cast<std::size_t>(letter)],
              word_index);
      ++counts[static_cast<std::size_t>(letter)];
    }
    for (std::size_t letter = 0; letter < k_alphabet_size; ++letter) {
      const std::size_t count = counts[letter];
      set_bit(m_letter_count_masks[letter][count], word_index);
    }
  }
}

std::size_t BitsetIndex::word_length() const { return m_word_length; }

std::size_t BitsetIndex::dictionary_size() const { return m_words.size(); }

FilterView BitsetIndex::all_words() const { return FilterView(*this, true); }

int BitsetIndex::char_to_index(char c) {
  if (c < 'a' || c > 'z') {
    throw std::invalid_argument(
        "words must contain only lower-case latin characters");
  }
  return c - 'a';
}

std::size_t BitsetIndex::bitset_word_count(std::size_t bit_count) {
  return (bit_count + 63) / 64;
}

BitsetIndex::Bitset BitsetIndex::full_mask() const {
  Bitset mask(m_bit_count, std::numeric_limits<std::uint64_t>::max());
  const std::size_t extra_bits = (m_bit_count * 64) - m_words.size();
  if (extra_bits > 0) {
    mask.back() >>= extra_bits;
  }
  return mask;
}

BitsetIndex::Bitset BitsetIndex::empty_mask() const {
  return Bitset(m_bit_count, 0);
}

} // namespace sieve

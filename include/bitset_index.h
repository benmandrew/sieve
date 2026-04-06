#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace sieve {

class BitsetIndex;

class FilterView {
public:
  FilterView(const BitsetIndex &index, bool all_candidates);

  void reset();
  void clear();

  void apply_feedback(std::string_view guess, std::string_view feedback);
  std::size_t candidate_count() const;
  bool is_candidate(std::size_t index) const;

  const std::string *next_candidate(std::size_t &cursor) const;

private:
  using letter_count_array = std::array<std::size_t, 26>;
  using letter_flag_array = std::array<bool, 26>;

  void initialize_letter_constraints(letter_count_array &min_count,
                                     letter_count_array &max_count,
                                     letter_flag_array &has_grey) const;
  void apply_position_constraints(std::string_view guess,
                                  std::string_view feedback,
                                  letter_count_array &min_count,
                                  letter_flag_array &has_grey);
  void apply_count_constraints(const letter_count_array &min_count,
                               const letter_count_array &max_count,
                               const letter_flag_array &has_grey);

  friend class BitsetIndex;

  const BitsetIndex &m_index;
  std::vector<std::uint64_t> m_candidates;
};

class BitsetIndex {
public:
  using Bitset = std::vector<std::uint64_t>;

  explicit BitsetIndex(std::vector<std::string> words);

  std::size_t word_length() const;
  std::size_t dictionary_size() const;

  FilterView all_words() const;

private:
  friend class FilterView;

  void validate_words_and_initialize_length();
  void build_lookup_masks();

  static constexpr int k_alphabet_size = 26;
  static constexpr char k_green = 'g';
  static constexpr char k_yellow = 'y';
  static constexpr char k_grey = 'b';

  static int char_to_index(char c);
  static std::size_t bitset_word_count(std::size_t bit_count);

  Bitset full_mask() const;
  Bitset empty_mask() const;

  std::vector<std::string> m_words;
  std::size_t m_word_length;
  std::size_t m_bit_count;

  std::vector<std::vector<Bitset>> m_position_letter_masks;
  std::vector<std::vector<Bitset>> m_letter_count_masks;
};

} // namespace sieve

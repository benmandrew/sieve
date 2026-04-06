#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace sieve {

/// Number of letters in the Latin alphabet.
inline constexpr std::size_t k_alphabet_size = 26;

class BitsetIndex;

/// Mutable filtered view over a BitsetIndex dictionary.
///
/// The view stores only a candidate bitset and never copies the underlying
/// dictionary words.
class FilterView {
  public:
    /// Construct a filter view for an index.
    ///
    /// @param index Backing dictionary index.
    /// @param all_candidates True to initialize with all words selected, false
    /// to initialize with no selected words.
    FilterView(const BitsetIndex &index, bool all_candidates);

    /// Reset candidates so every dictionary word is selectable.
    void reset();

    /// Clear candidates so no dictionary word is selectable.
    void clear();

    /// Apply Wordle feedback constraints to the current candidate set.
    ///
    /// Feedback symbols use g (green), y (yellow), and b (grey/black).
    ///
    /// @param guess Guess word with the same length as the dictionary words.
    /// @param feedback Feedback symbols for each guessed position.
    void apply_feedback(std::string_view guess, std::string_view feedback);

    /// Get the number of currently selected candidates.
    ///
    /// @return Number of words still matching applied constraints.
    std::size_t candidate_count() const;

    /// Check whether a dictionary entry is still a candidate.
    ///
    /// @param index Dictionary index to query.
    /// @return True if the word at index is still selected.
    bool is_candidate(std::size_t index) const;

    /// Get the next candidate word while advancing the cursor.
    ///
    /// @param cursor In/out cursor over dictionary indices.
    /// @return Pointer to the next candidate word, or nullptr when exhausted.
    const std::string *next_candidate(std::size_t &cursor) const;

  private:
    /// Fixed-size array of per-letter counts.
    using letter_count_array = std::array<std::size_t, k_alphabet_size>;

    /// Fixed-size array of per-letter flags.
    using letter_flag_array = std::array<bool, k_alphabet_size>;

    /// Initialize temporary min/max count constraints before evaluation.
    ///
    /// @param min_count Output lower bounds per letter.
    /// @param max_count Output upper bounds per letter.
    /// @param has_grey Output grey-marker flags per letter.
    void initialize_letter_constraints(letter_count_array &min_count,
                                       letter_count_array &max_count,
                                       letter_flag_array &has_grey) const;

    /// Apply per-position green/yellow/grey constraints.
    ///
    /// @param guess Guess word.
    /// @param feedback Feedback pattern.
    /// @param min_count In/out minimum letter counts derived from g/y markers.
    /// @param has_grey In/out grey-marker flags per letter.
    void apply_position_constraints(std::string_view guess,
                                    std::string_view feedback,
                                    letter_count_array &min_count,
                                    letter_flag_array &has_grey);

    /// Apply per-letter count-range constraints after positional filtering.
    ///
    /// @param min_count Minimum required count for each letter.
    /// @param max_count Maximum allowed count for each letter.
    /// @param has_grey Grey-marker flags used to tighten maxima.
    void apply_count_constraints(const letter_count_array &min_count,
                                 const letter_count_array &max_count,
                                 const letter_flag_array &has_grey);

    friend class BitsetIndex;

    /// Reference to the backing immutable index.
    const BitsetIndex &m_index;

    /// Candidate bitset for the current filtered view.
    std::vector<std::uint64_t> m_candidates;
};

/// Bitset-backed dictionary index for Wordle-like filtering.
///
/// The index precomputes position-letter and letter-count masks to support
/// fast candidate filtering via bitwise operations.
class BitsetIndex {
  public:
    /// Dynamic bitset storage type used by the index internals.
    using Bitset = std::vector<std::uint64_t>;

    /// Build an index from a dictionary.
    ///
    /// @param words Dictionary words (must be lowercase and equal-length).
    explicit BitsetIndex(std::vector<std::string> words);

    /// Get dictionary word length.
    ///
    /// @return Length of each indexed word.
    std::size_t word_length() const;

    /// Get number of indexed words.
    ///
    /// @return Total dictionary size.
    std::size_t dictionary_size() const;

    /// Create a filter view initialized with all words selected.
    ///
    /// @return A mutable filtering view over this index.
    FilterView all_words() const;

  private:
    friend class FilterView;

    /// Validate input words and initialize m_word_length.
    void validate_words_and_initialize_length();

    /// Build precomputed lookup masks used during filtering.
    void build_lookup_masks();

    /// Feedback marker for exact-position match.
    static constexpr char k_green = 'g';

    /// Feedback marker for present-letter wrong-position match.
    static constexpr char k_yellow = 'y';

    /// Feedback marker for absent-letter (or excess duplicate) match.
    static constexpr char k_grey = 'b';

    /// Convert a lowercase latin character to its alphabet index.
    ///
    /// @param c Input character.
    /// @return Zero-based alphabet index in [0, @ref sieve::k_alphabet_size -
    /// 1].
    static int char_to_index(char c);

    /// Compute number of uint64 words needed for a bitset.
    ///
    /// @param bit_count Number of bits to represent.
    /// @return Number of uint64 storage words required.
    static std::size_t bitset_word_count(std::size_t bit_count);

    /// Create a mask with all dictionary entries enabled.
    ///
    /// @return Full candidate mask.
    Bitset full_mask() const;

    /// Create a mask with no dictionary entries enabled.
    ///
    /// @return Empty candidate mask.
    Bitset empty_mask() const;

    /// Indexed dictionary words.
    std::vector<std::string> m_words;

    /// Common length of all words in m_words.
    std::size_t m_word_length;

    /// Number of uint64 slots needed for one candidate bitset.
    std::size_t m_bit_count;

    /// Per-position, per-letter masks.
    ///
    /// Outer index is position, inner array index is letter.
    std::vector<std::array<Bitset, k_alphabet_size>> m_position_letter_masks;

    /// Per-letter, per-count masks.
    ///
    /// Outer array index is letter, inner vector index is exact count.
    std::array<std::vector<Bitset>, k_alphabet_size> m_letter_count_masks;
};

}  // namespace sieve

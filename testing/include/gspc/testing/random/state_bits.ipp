#include <climits>
#include <random>



    namespace gspc::testing
    {
      template<> constexpr std::size_t state_bits<std::mt19937>()
      {
        return std::mt19937::state_size * std::mt19937::word_size;
      }
      template<> constexpr std::size_t state_bits<std::mt19937_64>()
      {
        return std::mt19937_64::state_size * std::mt19937_64::word_size;
      }
      template<> constexpr std::size_t state_bits<std::minstd_rand0>()
      {
        return sizeof (std::minstd_rand0::result_type) * CHAR_BIT;
      }
      template<> constexpr std::size_t state_bits<std::minstd_rand>()
      {
        return sizeof (std::minstd_rand::result_type) * CHAR_BIT;
      }
    }

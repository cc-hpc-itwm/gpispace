#include <net_description.hpp>
#include <nets_using_buffers.hpp>

#include <util-generic/print_container.hpp>
#include <util-generic/testing/random.hpp>

#include <boost/align/is_aligned.hpp>
#include <boost/format.hpp>

#include <string>

#include <algorithm>
#include <list>
#include <string>

namespace
{
  std::string get_new_buffer_name (std::list<std::string> const& buffers)
  {
    std::string buffer_name;

    do
    {
      buffer_name = fhg::util::testing::random_char_of
        ("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
      buffer_name += fhg::util::testing::random_string_of
        ("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_0123456789");
    } while ( std::find (buffers.begin(), buffers.end(), buffer_name)
            != buffers.end()
            );

    return buffer_name;
  }

  template<typename Alignment>
    std::string make_network ( unsigned long& size_without_align
                             , unsigned long& size_with_align
                             , Alignment&& alignment
                             )
  {
    std::string buffer_descriptions;
    std::list<std::string> buffer_names;
    std::string alignment_tests;

    unsigned long const num_buffers
      (fhg::util::testing::random<unsigned long>{} (15, 5));

    for (unsigned int i (0); i < num_buffers; ++i)
    {
      auto const buffer_name (get_new_buffer_name (buffer_names));
      auto const buffer_size
        (fhg::util::testing::random<unsigned long>{} (200, 100));
      boost::optional<unsigned long> const buffer_alignment (alignment());

      size_without_align += buffer_size;
      size_with_align += buffer_size + buffer_alignment.get_value_or (1) - 1;

      if (!buffer_alignment)
      {
        buffer_descriptions += create_buffer_description
          (buffer_name, buffer_size);
      }
      else
      {
        buffer_descriptions += create_buffer_description
          (buffer_name, buffer_size, buffer_alignment);
      }

      buffer_names.emplace_back (buffer_name);

      alignment_tests += create_alignment_test (buffer_alignment, buffer_name);
    }

    return create_net_description
      (buffer_descriptions, buffer_names, alignment_tests);
  }

  boost::none_t always_none()
  {
    return boost::none;
  }
  unsigned long random_power_of_two()
  {
    return 1ul << fhg::util::testing::random<unsigned long>{} (10, 0);
  }
}

std::string net_with_arbitrary_buffer_sizes_and_alignments
  (unsigned long& total_buffer_size)
{
  unsigned long ignore;
  return make_network (ignore, total_buffer_size, random_power_of_two);
}

std::string net_with_arbitrary_buffer_sizes_and_default_alignments
  (unsigned long& total_buffer_size)
{
  unsigned long ignore;
  return make_network (ignore, total_buffer_size, always_none);
}

std::string net_with_arbitrary_buffer_sizes_and_alignments_insufficient_memory
  (unsigned long& total_buffer_size)
{
  unsigned long ignore;
  return make_network (total_buffer_size, ignore, random_power_of_two);
}

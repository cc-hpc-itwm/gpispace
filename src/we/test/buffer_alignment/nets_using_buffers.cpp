#include <generate_buffer_names.hpp>
#include <net_description.hpp>
#include <nets_using_buffers.hpp>

#include <util-generic/print_container.hpp>
#include <util-generic/testing/random.hpp>

#include <boost/align/is_aligned.hpp>
#include <boost/format.hpp>

#include <string>

std::string net_with_arbitrary_buffer_sizes_and_alignments
  (unsigned long& total_buffer_size)
{
  std::string buffer_descriptions;
  std::list<std::string> buffer_names;
  std::string alignment_tests;

  unsigned long num_buffers
    (fhg::util::testing::random<unsigned long>{} (15, 5));

  for (unsigned int i (0); i < num_buffers; ++i)
  {
    auto const buffer_name (get_new_buffer_name (buffer_names));
    auto const buffer_size
      (fhg::util::testing::random<unsigned long>{} (200, 100));

    auto const exp
      (fhg::util::testing::random<unsigned long>{}(10,0));
    unsigned long const buffer_alignment (std::pow (2, exp));

    total_buffer_size += buffer_size + buffer_alignment - 1;

    buffer_descriptions += create_buffer_description 
                             (buffer_name, buffer_size, buffer_alignment);

    buffer_names.emplace_back (buffer_name);

    alignment_tests += create_alignment_test (buffer_alignment, buffer_name);
  }

  return create_net_description 
    (buffer_descriptions, buffer_names, alignment_tests);
}

std::string net_with_arbitrary_buffer_sizes_and_default_alignments
  (unsigned long& total_buffer_size)
{
  std::string buffer_descriptions;
  std::list<std::string> buffer_names;
  std::string alignment_tests;

  unsigned long const num_buffers
    (fhg::util::testing::random<unsigned long>{} (15, 5));

  unsigned int const default_alignment (1);

  for (unsigned int i (0); i < num_buffers; ++i)
  {
    auto const buffer_name (get_new_buffer_name (buffer_names));
    auto const buffer_size
      (fhg::util::testing::random<unsigned long>{} (200, 100));

    total_buffer_size += buffer_size;

    buffer_descriptions += create_buffer_description 
                             (buffer_name, buffer_size);
  
    buffer_names.emplace_back (buffer_name);

    alignment_tests += create_alignment_test (default_alignment, buffer_name);
  }

  return create_net_description 
    (buffer_descriptions, buffer_names, alignment_tests);
}

std::string net_with_arbitrary_buffer_sizes_and_alignments_insufficient_memory
  (unsigned long& total_buffer_size)
{
  std::string buffer_descriptions;
  std::list<std::string> buffer_names;
  std::string alignment_tests;

  unsigned long num_buffers
    (fhg::util::testing::random<unsigned long>{} (15, 5));

  for (unsigned int i (0); i < num_buffers; ++i)
  {
    auto const buffer_name (get_new_buffer_name (buffer_names));
    auto const buffer_size
      (fhg::util::testing::random<unsigned long>{} (200, 100));

    auto const exp
      (fhg::util::testing::random<unsigned long>{} (10, 0));
    unsigned long const buffer_alignment (std::pow (2, exp));

    total_buffer_size += buffer_size;

    buffer_descriptions += create_buffer_description
                             (buffer_name, buffer_size, buffer_alignment);

    buffer_names.emplace_back (buffer_name);

    alignment_tests += create_alignment_test (buffer_alignment, buffer_name);
  }

  return create_net_description 
    (buffer_descriptions, buffer_names, alignment_tests);
}

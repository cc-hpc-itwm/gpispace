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
  struct random_identifier_without_leading_underscore
  {
    std::string operator()() const
    {
      using underlying = fhg::util::testing::random<std::string>;
      return underlying{} (underlying::identifier_without_leading_underscore{});
    }
  };

  template<typename Alignment>
    std::string make_network ( unsigned long& size_without_align
                             , unsigned long& size_with_align
                             , Alignment&& alignment
                             )
  {
    size_with_align = 0;
    size_without_align = 0;

    fhg::util::testing::unique_random
      <std::string, random_identifier_without_leading_underscore> buffer_names;

    std::vector<BufferInfo> buffers
      (fhg::util::testing::random<std::size_t>{} (15, 5));

    for (auto& buffer : buffers)
    {
      buffer.name = buffer_names();
      buffer.size = fhg::util::testing::random<unsigned long>{} (200, 100);
      buffer.alignment = alignment();

      size_without_align += buffer.size;
      // Intentionally not align_up(): Don't assume base alignment, so
      // every case should be worst case.
      // \todo It isn't required to be worst case, is it? Probably is
      // mostly limited by biggest requirement and can thus be made a
      // more exact maximum needed size.
      size_with_align += buffer.size + buffer.alignment.get_value_or (1) - 1;
    }

    return create_net_description (buffers);
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

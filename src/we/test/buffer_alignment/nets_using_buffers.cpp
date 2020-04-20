#include <we/test/buffer_alignment/nets_using_buffers.hpp>

#include <we/test/buffer_alignment/net_description.hpp>

#include <util-generic/print_container.hpp>
#include <util-generic/testing/random.hpp>

#include <boost/align/is_aligned.hpp>
#include <boost/format.hpp>

#include <string>

#include <algorithm>
#include <list>
#include <string>

namespace we
{
  namespace test
  {
    namespace buffer_alignment
    {
      namespace
      {
        struct random_identifier_without_leading_underscore
        {
          std::string operator()() const
          {
            using underlying = fhg::util::testing::random<std::string>;
            return underlying{}
              (underlying::identifier_without_leading_underscore{});
          }
        };

        void align_up_worst
          (std::size_t& pos, boost::optional<std::size_t> align)
        {
          // Don't assume base alignment, so every case should be
          // worst case.
          // \todo It isn't required to be worst case to always
          // succeed, regardless of base alignment, is it? Probably is
          // mostly limited by biggest requirement and can thus be
          // made a more exact maximum worst-needed size.
          pos += align.get_value_or (1) - 1;
        }

        template<typename Alignment>
          std::string make_network ( unsigned long& size_without_align
                                   , unsigned long& size_with_align
                                   , Alignment&& alignment
                                   )
        {
          size_without_align = 0;
          size_with_align = 0;

          fhg::util::testing::unique_random
            <std::string, random_identifier_without_leading_underscore>
              buffer_names;

          std::vector<BufferInfo> buffers
            (fhg::util::testing::random<std::size_t>{} (15, 5));

          for (auto& buffer : buffers)
          {
            buffer.name = buffer_names();
            buffer.size
              = fhg::util::testing::random<unsigned long>{} (200, 100);
            buffer.alignment = alignment();

            align_up_worst (size_with_align, buffer.alignment);

            size_without_align += buffer.size;
            size_with_align += buffer.size;
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
        boost::optional<unsigned long> random_power_of_two_or_none()
        {
          auto const roll (random_power_of_two());
          return boost::make_optional (roll != 1, roll);
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

      std::string net_with_arbitrary_buffer_sizes_and_mixed_alignments
        (unsigned long& total_buffer_size)
      {
        unsigned long ignore;
        return make_network
          (ignore, total_buffer_size, random_power_of_two_or_none);
      }

      std::string net_with_arbitrary_buffer_sizes_and_alignments_insufficient_memory
        (unsigned long& size_without_align)
      {
        std::string result;
        unsigned long size_with_align;
        do
        {
          result = make_network
            (size_without_align, size_with_align, random_power_of_two);
        }
        while (size_without_align == size_with_align);
        return result;
      }
    }
  }
}

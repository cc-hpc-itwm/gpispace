// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

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
                                   , unsigned long& size_with_align_worst
                                   , Alignment&& alignment
                                   )
        {
          size_without_align = 0;
          size_with_align_worst = 0;

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

            align_up_worst (size_with_align_worst, buffer.alignment);

            size_without_align += buffer.size;
            size_with_align_worst += buffer.size;
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
          return boost::make_optional
            (fhg::util::testing::random<bool>{}(), random_power_of_two());
        }
      }

      std::string net_with_arbitrary_buffer_sizes_and_alignments
        (unsigned long& size_with_align_worst)
      {
        unsigned long size_without_align;
        return make_network ( size_without_align
                            , size_with_align_worst
                            , random_power_of_two
                            );
      }

      std::string net_with_arbitrary_buffer_sizes_and_default_alignments
        (unsigned long& size_with_align_worst)
      {
        unsigned long size_without_align;
        return make_network ( size_without_align
                            , size_with_align_worst
                            , always_none
                            );
      }

      std::string net_with_arbitrary_buffer_sizes_and_mixed_alignments
        (unsigned long& size_with_align_worst)
      {
        unsigned long size_without_align;
        return make_network ( size_without_align
                            , size_with_align_worst
                            , random_power_of_two_or_none
                            );
      }
    }
  }
}

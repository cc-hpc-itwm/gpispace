// Copyright (C) 2020-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <test/we/buffer_alignment/nets_using_buffers.hpp>

#include <test/we/buffer_alignment/net_description.hpp>

#include <gspc/util/print_container.hpp>
#include <gspc/testing/random.hpp>

#include <algorithm>
#include <list>
#include <string>



    namespace gspc::we::test::buffer_alignment
    {
      namespace
      {
        struct random_identifier_without_leading_underscore
        {
          std::string operator()() const
          {
            using underlying = gspc::testing::random<std::string>;
            return underlying{}
              (underlying::identifier_without_leading_underscore{});
          }
        };

        void align_up_worst
          (std::size_t& pos, std::optional<std::size_t> align)
        {
          // Don't assume base alignment, so every case should be
          // worst case.
          // \todo It isn't required to be worst case to always
          // succeed, regardless of base alignment, is it? Probably is
          // mostly limited by biggest requirement and can thus be
          // made a more exact maximum worst-needed size.
          pos += align.value_or (1) - 1;
        }

        template<typename Alignment>
          std::string make_network ( unsigned long& size_without_align
                                   , unsigned long& size_with_align_worst
                                   , Alignment&& alignment
                                   )
        {
          size_without_align = 0;
          size_with_align_worst = 0;

          gspc::testing::unique_random
            <std::string, random_identifier_without_leading_underscore>
              buffer_names;

          std::vector<BufferInfo> buffers
            (gspc::testing::random<std::size_t>{} (15, 5));

          for (auto& buffer : buffers)
          {
            buffer.name = buffer_names();
            buffer.size
              = gspc::testing::random<unsigned long>{} (200, 100);
            buffer.alignment = alignment();

            align_up_worst (size_with_align_worst, buffer.alignment);

            size_without_align += buffer.size;
            size_with_align_worst += buffer.size;
          }

          return create_net_description (buffers);
        }

        std::nullopt_t always_none()
        {
          return std::nullopt;
        }
        unsigned long random_power_of_two()
        {
          return 1ul << gspc::testing::random<unsigned long>{} (10, 0);
        }
        std::optional<unsigned long> random_power_of_two_or_none()
        {
          return gspc::testing::random<bool>{}()
            ? std::make_optional (random_power_of_two())
            : std::nullopt
            ;
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

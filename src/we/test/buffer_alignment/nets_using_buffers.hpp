// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>

namespace we
{
  namespace test
  {
    namespace buffer_alignment
    {
      std::string net_with_arbitrary_buffer_sizes_and_alignments
        (unsigned long& total_buffer_size);

      std::string net_with_arbitrary_buffer_sizes_and_default_alignments
        (unsigned long& total_buffer_size);

      std::string net_with_arbitrary_buffer_sizes_and_mixed_alignments
        (unsigned long& total_buffer_size);
    }
  }
}

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

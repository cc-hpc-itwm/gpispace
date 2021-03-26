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

#include <iml/gaspi/SegmentDescription.hpp>

namespace iml
{
  namespace gaspi
  {
    SegmentDescription::SegmentDescription
        ( std::size_t communication_buffer_size_
        , std::size_t communication_buffer_count_
        )
      : communication_buffer_size (communication_buffer_size_)
      , communication_buffer_count (communication_buffer_count_)
    {}
  }
}

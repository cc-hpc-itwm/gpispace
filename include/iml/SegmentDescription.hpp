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

#include <iml/beegfs/SegmentDescription.hpp>
#include <iml/gaspi/SegmentDescription.hpp>

#include <boost/variant/variant.hpp>

namespace iml
{
  //! Parameters for how to allocate a specific segment, depending on
  //! segment type.
  //! \see gaspi::SegmentDescription, beegfs::SegmentDescription,
  //! Client::create_segment()
  using SegmentDescription
    = ::boost::variant < gaspi::SegmentDescription
                     , beegfs::SegmentDescription
                     >;
}

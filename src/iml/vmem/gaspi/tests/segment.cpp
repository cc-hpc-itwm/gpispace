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

#include <iml/vmem/gaspi/pc/segment/segment.hpp>

#include <util-generic/finally.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>

#include <boost/test/unit_test.hpp>
#include <boost/lexical_cast.hpp>

#include <string>

BOOST_AUTO_TEST_CASE (sequence_of_create_close_unlink_does_not_throw)
{
  gpi::pc::segment::segment_t seg
    ( "seg-test-" + ::boost::lexical_cast<std::string>(getpid())
    , 1024
    );

  seg.create();
  FHG_UTIL_FINALLY ([&] { seg.unlink(); });

  seg.close();
}

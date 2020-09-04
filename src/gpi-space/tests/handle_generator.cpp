// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
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

#include <gpi-space/pc/memory/handle_generator.hpp>

#include <gpi-space/pc/type/segment_descriptor.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE ( test_generate )
{
  gpi::pc::memory::handle_generator_t handle_generator (42);

  gpi::pc::type::handle_t globl
      (handle_generator.next (gpi::pc::type::segment::SEG_GASPI));
  BOOST_CHECK_EQUAL (globl.type, gpi::pc::type::segment::SEG_GASPI);
  BOOST_CHECK_EQUAL (globl.gpi.ident, 42U);
  BOOST_CHECK_EQUAL (globl.gpi.cntr, 1U);

  gpi::pc::type::handle_t local
      (handle_generator.next (gpi::pc::type::segment::SEG_SHM));
  BOOST_CHECK_EQUAL (local.type, gpi::pc::type::segment::SEG_SHM);
  BOOST_CHECK_EQUAL (local.shm.cntr, 1U);

  gpi::pc::type::handle_id_t id = local;
  BOOST_CHECK_EQUAL (local, id);
}

BOOST_AUTO_TEST_CASE ( test_generate_interleaved )
{
  gpi::pc::memory::handle_generator_t handle_generator (42);

  for (size_t i (0); i < 100; ++i)
  {
    gpi::pc::type::handle_t g
        (handle_generator.next (gpi::pc::type::segment::SEG_GASPI));
    BOOST_CHECK_EQUAL (g.type, gpi::pc::type::segment::SEG_GASPI);
    BOOST_CHECK_EQUAL (g.gpi.ident, 42U);
    BOOST_CHECK_EQUAL (g.gpi.cntr, i+1);

    gpi::pc::type::handle_t s
        (handle_generator.next (gpi::pc::type::segment::SEG_SHM));
    BOOST_CHECK_EQUAL (s.type, gpi::pc::type::segment::SEG_SHM);
    BOOST_CHECK_EQUAL (s.shm.cntr, i+1);
  }
}

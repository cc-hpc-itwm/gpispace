// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iml/vmem/gaspi/pc/memory/handle_generator.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE ( generate_allocation )
{
  gpi::pc::memory::handle_generator_t handle_generator (0);

  BOOST_CHECK_EQUAL
    (handle_generator.next_allocation(), iml::AllocationHandle (1U));

  auto const local (handle_generator.next_allocation());
  BOOST_CHECK_EQUAL (local, iml::AllocationHandle (2U));
}

BOOST_AUTO_TEST_CASE ( generate_interleaved_allocation )
{
  gpi::pc::memory::handle_generator_t handle_generator (0);

  for (std::size_t i (0); i < 100; ++i)
  {
    BOOST_CHECK_EQUAL
      ( handle_generator.next_allocation()
      , iml::AllocationHandle (2*(i+1)-1)
      );
    BOOST_CHECK_EQUAL
      ( handle_generator.next_allocation()
      , iml::AllocationHandle (2*(i+1))
      );
  }
}

BOOST_AUTO_TEST_CASE ( generate_segment )
{
  gpi::pc::memory::handle_generator_t handle_generator (0);

  BOOST_CHECK_EQUAL
    (handle_generator.next_segment(), iml::SegmentHandle (1U));

  auto const local (handle_generator.next_segment());
  BOOST_CHECK_EQUAL (local, iml::SegmentHandle (2U));
}

BOOST_AUTO_TEST_CASE ( generate_interleaved_segment )
{
  gpi::pc::memory::handle_generator_t handle_generator (0);

  for (std::size_t i (0); i < 100; ++i)
  {
    BOOST_CHECK_EQUAL
      ( handle_generator.next_segment()
      , iml::SegmentHandle (2*(i+1)-1)
      );
    BOOST_CHECK_EQUAL
      ( handle_generator.next_segment()
      , iml::SegmentHandle (2*(i+1))
      );
  }
}

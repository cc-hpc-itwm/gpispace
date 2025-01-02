// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>
#include <boost/version.hpp>

#include <util-generic/procfs.hpp>
#include <util-generic/syscall.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>

BOOST_AUTO_TEST_CASE (entries)
{
  bool found_this_pid (false);

  for (auto const& entry : fhg::util::procfs::entries())
  {
    if (entry.pid() == fhg::util::syscall::getpid())
    {
      found_this_pid = true;

#if (BOOST_VERSION < 106000)

      BOOST_REQUIRE_EQUAL_COLLECTIONS
        ( entry.command_line().begin(), entry.command_line().end()
        , ::boost::unit_test::framework::master_test_suite().argv
        , ::boost::unit_test::framework::master_test_suite().argv
        + ::boost::unit_test::framework::master_test_suite().argc
        );

#else

      BOOST_REQUIRE_EQUAL
        ( entry.command_line().front()
        , ::boost::unit_test::framework::master_test_suite().argv[0]
        );

      BOOST_REQUIRE_EQUAL_COLLECTIONS
        ( std::next (std::find ( entry.command_line().begin()
                               , entry.command_line().end()
                               , "--"
                               )
                    )
        , entry.command_line().end()
        , std::next (::boost::unit_test::framework::master_test_suite().argv)
        , ::boost::unit_test::framework::master_test_suite().argv
        + ::boost::unit_test::framework::master_test_suite().argc
        );

#endif
    }
  }

  BOOST_REQUIRE (found_this_pid);
}

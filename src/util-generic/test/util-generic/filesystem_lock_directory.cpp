// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/filesystem_lock_directory.hpp>
#include <util-generic/temporary_path.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/require_exception.hpp>

#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE (lock_fails_on_existing_path_and_does_not_remove_it)
{
  fhg::util::temporary_path const path {::boost::filesystem::unique_path()};

  BOOST_REQUIRE (::boost::filesystem::exists (path));

  fhg::util::testing::require_exception
    ( [&path]
      {
        fhg::util::filesystem_lock_directory {path};
      }
    , fhg::util::testing::make_nested
        ( fhg::util::failed_to_create_lock (path)
        , std::logic_error
            ( ( ::boost::format ("Temporary path %1% already exists.")
              % static_cast<::boost::filesystem::path> (path)
              ).str()
            )
        )
    );

  BOOST_REQUIRE (::boost::filesystem::exists (path));
}

BOOST_AUTO_TEST_CASE (lock_creates_path_and_removes_it)
{
  ::boost::filesystem::path const path {::boost::filesystem::unique_path()};

  BOOST_REQUIRE (!::boost::filesystem::exists (path));

  {
    fhg::util::filesystem_lock_directory const lock1 (path);

    BOOST_REQUIRE (::boost::filesystem::exists (path));
  }

  BOOST_REQUIRE (!::boost::filesystem::exists (path));
}

BOOST_AUTO_TEST_CASE (second_lock_fails)
{
  ::boost::filesystem::path const path {::boost::filesystem::unique_path()};

  fhg::util::filesystem_lock_directory const lock (path);

  fhg::util::testing::require_exception
    ( [&path]
      {
        fhg::util::filesystem_lock_directory {path};
      }
    , fhg::util::testing::make_nested
        ( fhg::util::failed_to_create_lock (path)
        , std::logic_error
            ( ( ::boost::format ("Temporary path %1% already exists.")
              % path
              ).str()
            )
        )
    );
}

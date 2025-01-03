// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iml/Client.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/require_exception.hpp>

#include <boost/test/unit_test.hpp>

#include <stdexcept>
#include <string>

namespace iml
{
  BOOST_AUTO_TEST_CASE (using_a_nonexistent_socket_throws)
  {
    std::string const path ("/this/ is an hopefully/ non/-existent/ path");

    fhg::util::testing::require_exception
      ( [&]
        {
          Client {path};
        }
      , fhg::util::testing::make_nested
          ( std::runtime_error
              {"Failed to open IML communication socket '" + path + "'"}
          , ::boost::system::system_error
              { ::boost::system::errc::make_error_code
                  (::boost::system::errc::no_such_file_or_directory)
              }
          )
      );
  }
}

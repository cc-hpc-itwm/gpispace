// Copyright (C) 2020-2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/iml/Client.hpp>

#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/testing/require_exception.hpp>

#include <boost/system/system_error.hpp>
#include <boost/test/unit_test.hpp>

#include <stdexcept>
#include <string>

namespace gspc::iml
{
  BOOST_AUTO_TEST_CASE (using_a_nonexistent_socket_throws)
  {
    std::string const path ("/this/ is an hopefully/ non/-existent/ path");

    gspc::testing::require_exception
      ( [&]
        {
          Client {path};
        }
      , gspc::testing::make_nested
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

// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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

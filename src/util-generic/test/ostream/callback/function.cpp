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

#include <util-generic/callable_signature.hpp>
#include <util-generic/ostream/callback/function.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_type.hpp>

#include <boost/test/unit_test.hpp>

#include <sstream>

namespace fhg
{
  namespace util
  {
    namespace ostream
    {
      namespace callback
      {
        namespace
        {
          struct udt
          {
            int i;
          };
        }

        BOOST_AUTO_TEST_CASE
          (print_function_is_just_operator_ostream_as_callable)
        {
          BOOST_REQUIRE (( is_callable< print_function<int>
                                      , std::ostream& (std::ostream&, int)
                                      >::value
                        ));

          BOOST_REQUIRE (( is_callable< print_function<udt>
                                      , std::ostream& (std::ostream&, udt)
                                      >::value
                        ));
        }

        BOOST_AUTO_TEST_CASE
          (generic_binds_function_and_pipes_result_into_ostream)
        {
          std::size_t transform_called (0);
          generic<udt, std::string> const to_string
            ( [&] (udt const& i)
              {
                ++transform_called;
                return std::to_string (i.i + 1);
              }
            );


          auto value (testing::random<int>{}());
          std::ostringstream oss;
          to_string (oss, udt {value});

          BOOST_REQUIRE_EQUAL (oss.str(), std::to_string (value + 1));
          BOOST_REQUIRE_EQUAL (transform_called, 1);
        }

        BOOST_AUTO_TEST_CASE
          (select_is_an_adapter_to_do_the_same_with_a_pointer)
        {
          std::size_t transform_called (0);
          auto const to_string
            ( [&] (int const* i)
              {
                ++transform_called;
                return std::to_string (*i + 1);
              }
            );


          auto value (testing::random<int>{}());
          std::ostringstream oss;
          select<int, std::string> (to_string) (oss, value);

          BOOST_REQUIRE_EQUAL (oss.str(), std::to_string (value + 1));
          BOOST_REQUIRE_EQUAL (transform_called, 1);
        }

        BOOST_AUTO_TEST_CASE (id_is_id)
        {
          auto value (testing::random<int>{}());
          std::ostringstream oss;
          id<int>() (oss, value);

          BOOST_REQUIRE_EQUAL (oss.str(), std::to_string (value));
        }
      }
    }
  }
}

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

#include <util-generic/ostream/modifier.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/random.hpp>

#include <boost/test/unit_test.hpp>

#include <sstream>
#include <string>

namespace
{
  class modifier_with_state : public fhg::util::ostream::modifier
  {
  public:
    modifier_with_state (std::string const& prefix)
      : _prefix (prefix)
      , _state (0)
    {}
    virtual std::ostream& operator() (std::ostream& os) const override
    {
      return os << _prefix << _state++;
    }
  private:
    std::string const _prefix;
    mutable unsigned int _state;
  };
}

BOOST_AUTO_TEST_CASE (output_calls_operator)
{
  std::string const prefix {fhg::util::testing::random<std::string>()()};
  modifier_with_state const modifier (prefix);

  auto&& verify
    ([&prefix, &modifier] (std::string const& expected)
     {
       std::ostringstream oss;
       oss << modifier;
       BOOST_REQUIRE_EQUAL (oss.str(), prefix + expected);
     }
    );

  verify ("0");
  verify ("1");
  verify ("2");
}

BOOST_AUTO_TEST_CASE (string_calls_operator)
{
  std::string const prefix {fhg::util::testing::random<std::string>()()};
  modifier_with_state const modifier (prefix);

  auto&& verify
    ([&prefix, &modifier] (std::string const& expected)
     {
       BOOST_REQUIRE_EQUAL (modifier.string(), prefix + expected);
     }
    );

  verify ("0");
  verify ("1");
  verify ("2");
}

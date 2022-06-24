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

#include <preferences_and_multimodules/WorkflowResult.hpp>

#include <we/signature_of.hpp>
#include <we/type/signature/show.hpp>
#include <we/type/value/show.hpp>

#include <util-generic/cxx17/holds_alternative.hpp>
#include <util-generic/join.hpp>

#include <boost/format.hpp>

#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>

namespace preferences_and_multimodules
{
  auto const error_inconsistent_type
    { [] (auto const& value, auto const& type_description)
      {
        throw std::logic_error
          ( ( boost::format ("Inconsistency: Expected type '%1%'. "
                             "Got value '%2%' with signature '%3%'."
                            )
            % type_description
            % pnet::type::value::show (value)
            % pnet::type::signature::show (pnet::signature_of (value))
            ).str()
          );
      }
    };

  void WorkflowResult::assert_key_count
    ( Key key
    , std::size_t expected_count
    ) const
  {
    auto const count (_values_on_ports.count (key));

    if (count != expected_count)
    {
      throw std::logic_error
        (str ( boost::format ("Expected count '%1%' for key '%2%': "
                              "Got count '%3%' in { %4% }"
                             )
              % expected_count
              % key
              % count
              % fhg::util::join
                  ( _values_on_ports, ","
                  , [] (auto& os, auto const& kv) -> decltype (os)
                    {
                      return os << kv.first
                                << " = "
                                << pnet::type::value::show (kv.second);
                    }
                  )
              )
        );
    }
  }

  template<typename T, typename TypeDescription>
    T const& WorkflowResult::get_impl
      (Key key, TypeDescription type_description) const
  {
    assert_key_count (key, 1);

    auto const& value (_values_on_ports.find (key)->second);

    if (!fhg::util::cxx17::holds_alternative<T> (value))
    {
      error_inconsistent_type (value, type_description);
    }

    return boost::get<T> (value);
  }

  template<>
    we::type::literal::control const& WorkflowResult::get (Key key) const
  {
    return get_impl<we::type::literal::control> (key, "control");
  }

  template<typename T, typename TypeDescription>
    std::vector<std::string> WorkflowResult::get_all_impl
      (Key key, TypeDescription type_description, std::size_t expected_count) const
  {
    assert_key_count (key, expected_count);

    auto const range (_values_on_ports.equal_range (key));

    std::vector<std::string> values;
    std::transform
      ( range.first
      , range.second
      , std::back_inserter (values)
      , [&type_description] (auto const& key_and_value)
        {
          auto const& value (key_and_value.second);
          if (!fhg::util::cxx17::holds_alternative<T> (value))
          {
            error_inconsistent_type (value, type_description);
          }

          return ::boost::get<std::string> (value);
        }
      );

    return values;
  }

  template<>
    std::vector<std::string> WorkflowResult::get_all
      (Key key, std::size_t expected_count) const
  {
    return get_all_impl<std::string> (key, "string", expected_count);
  }
}

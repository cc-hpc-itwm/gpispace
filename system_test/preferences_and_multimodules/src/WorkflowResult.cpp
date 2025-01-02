// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <preferences_and_multimodules/WorkflowResult.hpp>

#include <we/signature_of.hpp>
#include <we/type/signature/show.hpp>
#include <we/type/value/show.hpp>

#include <util-generic/cxx17/holds_alternative.hpp>
#include <util-generic/join.hpp>

#include <fmt/core.h>
#include <fmt/ostream.h>
#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>

namespace fmt
{
  template<> struct formatter<pnet::type::value::show> : ostream_formatter{};
  template<> struct formatter<pnet::type::signature::show> : ostream_formatter{};
}

namespace preferences_and_multimodules
{
  auto const error_inconsistent_type
    { [] (auto const& value, auto const& type_description)
      {
        throw std::logic_error
          { fmt::format
              ( "Inconsistency: Expected type '{}'. "
                "Got value '{}' with signature '{}'."
              , type_description
              , pnet::type::value::show (value)
              , pnet::type::signature::show (pnet::signature_of (value))
              )
          };
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
        { fmt::format
            ( "Expected count '{}' for key '{}': "
              "Got count '{}' in {{ {} }}"
            , expected_count
            , key
            , count
            , fhg::util::join
                ( _values_on_ports, ","
                , [] (auto& os, auto const& kv) -> decltype (os)
                  {
                    return os << kv.first
                              << " = "
                              << pnet::type::value::show (kv.second);
                  }
                ).string()
            )
        };
    }
  }

  template<typename T, typename TypeDescription>
    T const& WorkflowResult::at_implementation
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
    we::type::literal::control const& WorkflowResult::at (Key key) const
  {
    return at_implementation<we::type::literal::control> (key, "control");
  }

  template<typename T, typename TypeDescription>
    std::vector<std::string> WorkflowResult::at_all_implementation
      ( Key key
      , TypeDescription type_description
      , std::size_t expected_count
      ) const
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
    std::vector<std::string> WorkflowResult::at_all
      (Key key, std::size_t expected_count) const
  {
    return at_all_implementation<std::string> (key, "string", expected_count);
  }
}

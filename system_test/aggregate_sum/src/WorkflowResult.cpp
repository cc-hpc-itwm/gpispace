// Copyright (C) 2021,2023-2024,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <aggregate_sum/WorkflowResult.hpp>

#include <gspc/we/signature_of.hpp>
#include <gspc/we/type/signature/show.hpp>
#include <gspc/we/type/value/show.hpp>

#include <gspc/util/cxx17/holds_alternative.hpp>
#include <gspc/util/join.hpp>

#include <fmt/core.h>
#include <fmt/ostream.h>
#include <stdexcept>

namespace fmt
{
  template<> struct formatter<gspc::pnet::type::value::show> : ostream_formatter{};
  template<> struct formatter<gspc::pnet::type::signature::show> : ostream_formatter{};
}

namespace aggregate_sum
{
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
            ( "Expected count '{}' for key '{}': Got count '{}' in {{ {} }}"
            , expected_count
            , key
            , count
            , gspc::util::join
              ( _values_on_ports, ","
              , [] (auto& os, auto const& kv) -> decltype (os)
                {
                  return os << kv.first << " = " << gspc::pnet::type::value::show (kv.second);
                }
              ).string()
            )
        };
    }
  }

  template<typename T, typename TypeDescription>
    T const& WorkflowResult::at_implementation
      ( Key key
      , TypeDescription type_description
      ) const
  {
    assert_key_count (key, 1);

    auto const& value (_values_on_ports.find (key)->second);

    if (!gspc::util::cxx17::holds_alternative<T> (value))
    {
      throw std::logic_error
        { fmt::format
            ( "Inconsistency: Expected type '{}'. Got value '{}' with signature '{}'."
            , type_description
            , gspc::pnet::type::value::show (value)
            , gspc::pnet::type::signature::show (gspc::pnet::signature_of (value))
            )
        };
    }

    return boost::get<T> (value);
  }

  template<> int const& WorkflowResult::at (Key key) const
  {
    return at_implementation<int> (key, "int");
  }
}

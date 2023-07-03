// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <aggregate_sum/WorkflowResult.hpp>

#include <we/signature_of.hpp>
#include <we/type/signature/show.hpp>
#include <we/type/value/show.hpp>

#include <util-generic/cxx17/holds_alternative.hpp>
#include <util-generic/join.hpp>

#include <boost/format.hpp>

#include <stdexcept>

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
        (str ( boost::format ("Expected count '%1%' for key '%2%': Got count '%3%' in { %4% }")
              % expected_count
              % key
              % count
              % fhg::util::join
                ( _values_on_ports, ","
                , [] (auto& os, auto const& kv) -> decltype (os)
                  {
                    return os << kv.first << " = " << pnet::type::value::show (kv.second);
                  }
                )
              )
        );
    }
  }

  template<typename T, typename TypeDescription>
    T const& WorkflowResult::get_impl (Key key, TypeDescription type_description) const
  {
    assert_key_count (key, 1);

    auto const& value (_values_on_ports.find (key)->second);

    if (!fhg::util::cxx17::holds_alternative<T> (value))
    {
      throw std::logic_error
        (str ( boost::format ("Inconsistency: Expected type '%1%'. Got value '%2%' with signature '%3%'.")
             % type_description
             % pnet::type::value::show (value)
             % pnet::type::signature::show (pnet::signature_of (value))
             )
        );
    }

    return boost::get<T> (value);
  }

  template<> int const& WorkflowResult::get (Key key) const
  {
    return get_impl<int> (key, "int");
  }
}

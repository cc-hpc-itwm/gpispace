// Copyright (C) 2021,2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/we/type/value.hpp>

#include <map>
#include <string>

namespace aggregate_sum
{
  class ValuesOnPorts
  {
  public:
     using Key = std::string;
     using Value = gspc::pnet::type::value::value_type;
     using Map = std::multimap<Key, Value>;

     ValuesOnPorts (Map map);

     Map const& map() const;

  protected:
     Map _values_on_ports;
  };
}

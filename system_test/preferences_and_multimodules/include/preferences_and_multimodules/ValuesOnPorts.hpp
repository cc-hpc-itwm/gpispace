// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <we/type/value.hpp>

#include <map>
#include <string>

namespace preferences_and_multimodules
{
  class ValuesOnPorts
  {
  public:
     using Key = std::string;
     using Value = pnet::type::value::value_type;
     using Map = std::multimap<Key, Value>;

     ValuesOnPorts (Map map);

     Map const& map() const;

  protected:
     Map _values_on_ports;
  };
}

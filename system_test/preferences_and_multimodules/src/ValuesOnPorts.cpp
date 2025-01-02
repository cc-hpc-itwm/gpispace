// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <preferences_and_multimodules/ValuesOnPorts.hpp>

namespace preferences_and_multimodules
{
  ValuesOnPorts::ValuesOnPorts (Map map)
    : _values_on_ports (map)
  {}

  ValuesOnPorts::Map const& ValuesOnPorts::map() const
  {
     return _values_on_ports;
  }
}

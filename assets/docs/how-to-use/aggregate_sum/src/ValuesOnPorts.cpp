#include <aggregate_sum/ValuesOnPorts.hpp>

namespace aggregate_sum
{
  ValuesOnPorts::ValuesOnPorts (Map map) : _values_on_ports (map) {}

  ValuesOnPorts::Map const& ValuesOnPorts::map() const
  {
     return _values_on_ports;
  }
}

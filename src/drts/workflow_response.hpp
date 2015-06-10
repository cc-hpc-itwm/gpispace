#include <we/type/value.hpp>

#include <string>

namespace gspc
{
  void workflow_response ( std::string const& trigger_address
                         , unsigned short const& trigger_port
                         , pnet::type::value::value_type const& value
                         );
}

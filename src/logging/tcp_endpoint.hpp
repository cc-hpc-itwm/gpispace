#pragma once

#include <boost/serialization/utility.hpp>

#include <string>
#include <utility>

namespace fhg
{
  namespace logging
  {
    using tcp_endpoint = std::pair<std::string, unsigned short>;
  }
}

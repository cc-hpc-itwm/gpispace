#pragma once

#include <stdexcept>
#include <string>

namespace fhg
{
  namespace util
  {
    std::string make_backtrace (const std::string& reason);

    class backtracing_exception : public std::runtime_error
    {
    public:
      backtracing_exception (const std::string& reason);
    };
  }
}

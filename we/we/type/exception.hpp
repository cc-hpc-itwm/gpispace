// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_EXCEPTION_HPP
#define _WE_EXCEPTION_HPP

#include <stdexcept>
#include <string>

namespace petri_net
{
  namespace exception
  {
    class generic : public std::runtime_error
    {
    public:
      explicit generic (const std::string& msg)
        : std::runtime_error ("net: " + msg)
      {}
    };

    class transition_not_enabled : public generic
    {
    public:
      explicit transition_not_enabled (const std::string & msg)
        : generic ("transition_not_enabled: " + msg)
      {}
    };

    class no_such : public generic
    {
    public:
      explicit no_such (const std::string & msg)
        : generic ("no_such: " + msg)
      {}
    };
  }
} // namespace petri_net

#endif

// mirko.rahn@itwm.fraunhofer.de

#ifndef _PETRI_NET_EXCEPTION_HPP_376a13b2_185b_401f_afe7_822af772d382
#define _PETRI_NET_EXCEPTION_HPP_376a13b2_185b_401f_afe7_822af772d382

#include <stdexcept>

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

#endif // _PETRI_NET_EXCEPTION_HPP_376a13b2_185b_401f_afe7_822af772d382

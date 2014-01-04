#ifndef GSPC_NET_RESOLVER_HPP
#define GSPC_NET_RESOLVER_HPP

#include <string>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>

namespace gspc
{
  namespace net
  {
    template <class Proto>
    struct resolver
    {
      typedef typename Proto::endpoint endpoint_type;

      static
      endpoint_type resolve ( std::string const &address
                            , boost::system::error_code &ec
                            )
      {
        try
        {
          ec = boost::system::errc::make_error_code
            (boost::system::errc::success);
          return resolve (address);
        }
        catch (const boost::system::system_error& ex)
        {
          ec = ex.code();
          return endpoint_type();
        }
      }

      static
      endpoint_type resolve (std::string const &address);
    };
  }
}

#endif

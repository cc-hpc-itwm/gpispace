// bernd.loerwald@itwm.fraunhofer.de

#include <fhg/util/boost/asio/ip/address.hpp>

#include <util-generic/hostname.hpp>

namespace fhg
{
  namespace util
  {
    std::string connectable_to_address_string
      (boost::asio::ip::address const& address)
    {
      if (address.is_unspecified())
      {
        return hostname();
      }
      return address.to_string();
    }
  }
}

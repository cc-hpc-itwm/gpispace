// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_UTIL_BOOST_ASIO_IP_ADDRESS_HPP
#define FHG_UTIL_BOOST_ASIO_IP_ADDRESS_HPP

#include <boost/asio/ip/address.hpp>

#include <string>

namespace fhg
{
  namespace util
  {
    std::string connectable_to_address_string (boost::asio::ip::address const&);
  }
}

#endif

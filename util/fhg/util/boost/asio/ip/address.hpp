// bernd.loerwald@itwm.fraunhofer.de

#pragma once

#include <boost/asio/ip/address.hpp>

#include <string>

namespace fhg
{
  namespace util
  {
    std::string connectable_to_address_string (boost::asio::ip::address const&);
  }
}

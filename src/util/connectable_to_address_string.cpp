#include <gspc/util/connectable_to_address_string.hpp>

#include <gspc/util/hostname.hpp>


  namespace gspc::util
  {
    std::string connectable_to_address_string
      (::boost::asio::ip::address const& address)
    {
      if (address.is_unspecified())
      {
        return hostname();
      }
      return address.to_string();
    }
  }

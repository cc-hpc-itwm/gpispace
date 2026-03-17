
  namespace gspc::util
  {
    template<typename BoostAsioIpEndpoint>
      std::pair<std::string, unsigned short>
        connectable_to_address_string (BoostAsioIpEndpoint endpoint)
    {
      return { connectable_to_address_string (endpoint.address())
             , endpoint.port()
             };
    }
  }

// bernd.loerwald@itwm.fraunhofer.de

#include <boost/asio/ip/tcp.hpp>
#include <boost/serialization/split_free.hpp>

#include <functional>
#include <string>

namespace boost
{
  namespace serialization
  {
    template<typename Archive>
      void load ( Archive& ar
                , boost::asio::ip::tcp::endpoint& ep
                , const unsigned int
                )
    {
      std::string address;
      unsigned short port;
      ar & address;
      ar & port;
      ep = boost::asio::ip::tcp::endpoint
        (boost::asio::ip::address::from_string (address), port);
    }
    template<typename Archive>
      void save ( Archive& ar
                , boost::asio::ip::tcp::endpoint const& ep
                , const unsigned int
                )
    {
      std::string const address (ep.address().to_string());
      unsigned short const port (ep.port());
      ar & address;
      ar & port;
    }
  }
}

BOOST_SERIALIZATION_SPLIT_FREE (boost::asio::ip::tcp::endpoint)

namespace std
{
  template<>
    struct hash<boost::asio::ip::tcp::endpoint>
  {
    typedef boost::asio::ip::tcp::endpoint argument_type;
    typedef std::size_t result_type;

    result_type operator() (argument_type const& endpoint) const
    {
      //! \todo actual hashing
      return 0;
    }
  };
}
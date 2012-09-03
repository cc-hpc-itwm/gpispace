#ifndef FHG_COM_KVS_MESSAGE_PING_HPP
#define FHG_COM_KVS_MESSAGE_PING_HPP 1

#include <boost/lexical_cast.hpp>

// serialization
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/variant.hpp>

#include <string>

namespace fhg
{
  namespace com
  {
    namespace kvs
    {
      namespace message
      {
        struct msg_ping
        {
          msg_ping () {}

          bool operator==(const msg_ping & rhs) const
          {
            return true;
          }
        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /* version */ )
          {
          }
        };
      }
    }
  }
}

#endif

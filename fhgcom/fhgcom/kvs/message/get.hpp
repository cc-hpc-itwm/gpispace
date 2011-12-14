#ifndef FHG_COM_KVS_MESSAGE_GET_HPP
#define FHG_COM_KVS_MESSAGE_GET_HPP 1

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
        struct msg_get
        {
          msg_get ()
          {}

          explicit
          msg_get (const std::string & k)
            : key_(k)
          {}

          const std::string & key () const { return key_; }
          bool operator==(const msg_get & rhs) const
          {
            return key_ == rhs.key_;
          }
        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /* version */ )
          {
            ar & BOOST_SERIALIZATION_NVP( key_ );
          }

          std::string key_;
        };
      }
    }
  }
}

#endif

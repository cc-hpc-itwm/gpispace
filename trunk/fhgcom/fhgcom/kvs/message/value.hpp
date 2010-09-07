#ifndef FHG_COM_KVS_MESSAGE_VALUE_HPP
#define FHG_COM_KVS_MESSAGE_VALUE_HPP 1

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
        struct value
        {
          value () {}

          explicit
          value (const std::string & v)
            : val_(v)
          {}

          const std::string & val () const { return val_; }
          bool operator==(const value & rhs) const
          {
            return val_ == rhs.val_;
          }
        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /* version */ )
          {
            ar & BOOST_SERIALIZATION_NVP( val_ );
          }

          std::string val_;
        };
      }
    }
  }
}

#endif

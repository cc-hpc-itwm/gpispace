#ifndef FHG_COM_KVS_MESSAGE_PUT_HPP
#define FHG_COM_KVS_MESSAGE_PUT_HPP 1

#include <boost/lexical_cast.hpp>

// serialization
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/variant.hpp>

#include <string>
#include <map>

namespace fhg
{
  namespace com
  {
    namespace kvs
    {
      namespace message
      {
        struct put
        {
          typedef std::map<std::string, std::string> map_type;

          put ()
            : expiry_ (0)
          {}

          explicit
          put (const std::string & k, const std::string & v)
            : expiry_ (0)
          {
            add (k, v);
          }

          template <typename T>
          explicit
          put (const std::string & k, T t)
            : expiry_ (0)
          {
            add (k, boost::lexical_cast<std::string>(t));
          }

          explicit
          put (const map_type & e)
            : entries_(e)
            , expiry_ (0)
          {}

          const map_type & entries () const { return entries_; }
          size_t expiry () const { return expiry_; }

          put & set_expiry (size_t exp)
          {
            expiry_ = exp;
            return *this;
          }

          void add (std::string const & key, std::string const & val)
          {
            entries_[key] = val;
          }

          bool operator==(const put & rhs) const
          {
            return entries_ == rhs.entries_;
          }
        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /* version */ )
          {
            ar & BOOST_SERIALIZATION_NVP ( entries_ );
            ar & BOOST_SERIALIZATION_NVP ( expiry_ );
          }

          map_type entries_;
          size_t expiry_;
        };
      }
    }
  }
}

#endif

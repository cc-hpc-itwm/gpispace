#ifndef FHG_COM_KVS_MESSAGE_LIST_HPP
#define FHG_COM_KVS_MESSAGE_LIST_HPP 1

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
        struct list
        {
          typedef std::map<std::string, std::string> map_type;

          list () {}

          const map_type & entries () const { return entries_; }
          map_type & entries () { return entries_; }

          bool operator==(const list & rhs) const
          {
            map_type::const_iterator i1 (entries_.begin());
            map_type::const_iterator i2 (rhs.entries_.begin());

            while (i1 != entries_.end() && i2 != rhs.entries_.end())
            {
              if (*i1 != *i2) return false;

              ++i1;
              ++i2;
            }

            if (i1 != entries_.end() || i2 != rhs.entries_.end())
              return false;

            return true;
          }
        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /* version */ )
          {
            ar & BOOST_SERIALIZATION_NVP( entries_ );
          }

          map_type entries_;
        };
      }
    }
  }
}

#endif

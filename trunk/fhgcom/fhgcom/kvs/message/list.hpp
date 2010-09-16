#ifndef FHG_COM_KVS_MESSAGE_LIST_HPP
#define FHG_COM_KVS_MESSAGE_LIST_HPP 1

#include <boost/lexical_cast.hpp>

// serialization
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/variant.hpp>

#include <string>
#include <set>

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
          list () {}

          const std::set<std::string> & keys () const { return keys_; }
          std::set<std::string> & keys () { return keys_; }

          void insert (std::string const & k)
          {
            keys_.insert (k);
          }
          void erase (std::string const & k)
          {
            keys_.erase (k);
          }

          bool operator==(const list & rhs) const
          {
            typedef std::set<std::string> set_type;
            set_type::const_iterator i1 (keys_.begin());
            set_type::const_iterator i2 (rhs.keys_.begin());

            while (i1 != keys_.end() && i2 != rhs.keys_.end())
            {
              if (*i1 != *i2) return false;

              ++i1;
              ++i2;
            }

            if (i1 != keys_.end() || i2 != rhs.keys_.end())
              return false;

            return true;
          }
        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /* version */ )
          {
            ar & BOOST_SERIALIZATION_NVP( keys_ );
          }

          std::set<std::string> keys_;
        };

        struct req_list
        {
          req_list () {}

          explicit req_list (const std::string & regexp)
            : regexp_(regexp)
          {}

          std::string const & regexp() const { return regexp_; }

          bool operator==(const req_list & rhs) const
          {
            return regexp_ == rhs.regexp_;
          }
        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /* version */ )
          {
            ar & regexp_;
          }

          std::string regexp_;
        };
      }
    }
  }
}

#endif

#ifndef FHG_COM_KVS_MESSAGE_REQ_LIST_HPP
#define FHG_COM_KVS_MESSAGE_REQ_LIST_HPP 1

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

#ifndef FHG_COM_KVS_MESSAGE_LOAD_HPP
#define FHG_COM_KVS_MESSAGE_LOAD_HPP 1

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
        struct msg_load
        {
          msg_load () {}

          explicit
          msg_load (const std::string & file)
            : file_(file)
          {}

          const std::string & file () const { return file_; }
          bool operator==(const msg_load & rhs) const
          {
            return file_ == rhs.file_;
          }
        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /* version */ )
          {
            ar & BOOST_SERIALIZATION_NVP( file_ );
          }

          std::string file_;
        };
      }
    }
  }
}

#endif

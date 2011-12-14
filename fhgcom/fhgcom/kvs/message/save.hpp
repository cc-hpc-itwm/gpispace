#ifndef FHG_COM_KVS_MESSAGE_SAVE_HPP
#define FHG_COM_KVS_MESSAGE_SAVE_HPP 1

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
        struct msg_save
        {
          msg_save () {}

          explicit
          msg_save (const std::string & file)
            : file_(file)
          {}

          const std::string & file () const { return file_; }
          bool operator==(const msg_save & rhs) const
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

#ifndef FHG_COM_KVS_MESSAGE_ERROR_HPP
#define FHG_COM_KVS_MESSAGE_ERROR_HPP 1

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
        struct error
        {
          enum code
            {
                KVS_ENOERROR = 0
              , KVS_ENOSUCH
              , KVS_EPERM
              , KVS_EINVAL
              , KVS_EUNKNOWN
            };

          error ()
            : code_ (KVS_ENOERROR)
          {}

          explicit
          error (code ec, const std::string & msg)
            : code_(ec)
            , message_(msg)
          {}

          code ec () const
          {
            return code_;
          }

          const char * what () const { return message_.c_str(); }
          bool operator==(const error & rhs) const
          {
            return message_ == rhs.message_;
          }

          static std::string code_to_string (code ec)
          {
            switch (ec)
            {
            case KVS_ENOERROR:
              return "ok";
            case KVS_ENOSUCH:
              return "no such";
            case KVS_EPERM:
              return "permission denied";
            case KVS_EINVAL:
              return "invalid argument";
            case KVS_EUNKNOWN:
              return "unknown error";
            default:
              throw std::runtime_error ("STRANGE! error code not handled!");
            }
          }
        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /* version */ )
          {
            ar & BOOST_SERIALIZATION_NVP( code_ );
            ar & BOOST_SERIALIZATION_NVP( message_ );
          }

          code code_;
          std::string message_;
        };
      }
    }
  }
}

#endif

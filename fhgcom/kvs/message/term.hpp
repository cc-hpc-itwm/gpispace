#ifndef FHG_COM_KVS_MESSAGE_TERM_HPP
#define FHG_COM_KVS_MESSAGE_TERM_HPP 1

// serialization
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/variant.hpp>

#include <string>
#include <unistd.h>
#include <sys/types.h>

namespace fhg
{
  namespace com
  {
    namespace kvs
    {
      namespace message
      {
        struct msg_term
        {
          msg_term () {}

          explicit
          msg_term (int ec, std::string const &rsn)
            : m_code (ec)
            , m_reason (rsn)
            , m_uid (getuid ())
          {}

          const std::string & reason () const { return m_reason; }
          int code () const { return m_code; }
          uid_t uid () const { return m_uid; }

          bool operator==(const msg_term & rhs) const
          {
            return (m_reason == rhs.m_reason)
              &&   (m_code == rhs.m_code);

          }
        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /* version */ )
          {
            ar & BOOST_SERIALIZATION_NVP( m_code );
            ar & BOOST_SERIALIZATION_NVP( m_reason );
            ar & BOOST_SERIALIZATION_NVP( m_uid );
          }

          int m_code;
          std::string m_reason;
          uid_t m_uid;
        };
      }
    }
  }
}

#endif

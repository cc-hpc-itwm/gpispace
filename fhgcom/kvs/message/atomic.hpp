#ifndef FHG_COM_KVS_MESSAGE_ATOMIC_HPP
#define FHG_COM_KVS_MESSAGE_ATOMIC_HPP 1

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
        struct msg_inc
        {
          msg_inc ()
          {}

          explicit
          msg_inc (const std::string & k, int step=1)
            : key_(k)
            , m_step (step)
          {}

          const std::string & key () const { return key_; }
          const int & step () const { return m_step; }
          bool operator==(const msg_inc & rhs) const
          {
            return key_ == rhs.key_ && m_step == rhs.m_step;
          }
        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /* version */ )
          {
            ar & BOOST_SERIALIZATION_NVP( key_ );
            ar & BOOST_SERIALIZATION_NVP( m_step );
          }

          std::string key_;
          int m_step;
        };
      }
    }
  }
}

#endif

#ifndef GPI_SPACE_PC_PROTO_CONTROL_HPP
#define GPI_SPACE_PC_PROTO_CONTROL_HPP 1

#include <ostream>

#include <boost/variant.hpp>

#include <gpi-space/pc/type/typedefs.hpp>
#include <gpi-space/pc/type/info_descriptor.hpp>

// serialization
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/variant.hpp>

namespace gpi
{
  namespace pc
  {
    namespace proto
    {
      namespace control
      {
        struct ping_t
        {
        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /*version*/)
          {}
        };

        struct pong_t
        {
        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /*version*/)
          {}
        };

        struct info_t
        {
        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /*version*/)
          {}
        };

        struct info_reply_t
        {
          gpi::pc::type::info::descriptor_t info;

        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /*version*/)
          {
            ar & info;
          }
        };

        typedef boost::variant<ping_t, pong_t, info_t, info_reply_t> message_t;
      }
    }
  }
}

#endif

#ifndef GPI_SPACE_PC_PROTO_SEGMENT_HPP
#define GPI_SPACE_PC_PROTO_SEGMENT_HPP 1

// serialization
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/variant.hpp>

#include <gpi-space/pc/type/typedefs.hpp>
#include <gpi-space/pc/type/segment_descriptor.hpp>

namespace gpi
{
  namespace pc
  {
    namespace proto
    {
      namespace segment
      {
        struct register_t
        {
          gpi::pc::type::name_t  name;
          gpi::pc::type::size_t  size;
          gpi::pc::type::flags_t flags;

        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /*version*/)
          {
            ar & BOOST_SERIALIZATION_NVP( name );
            ar & BOOST_SERIALIZATION_NVP( size );
            ar & BOOST_SERIALIZATION_NVP( flags );
          }
        };

        struct register_reply_t
        {
          gpi::pc::type::segment_id_t id;

        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /*version*/)
          {
            ar & BOOST_SERIALIZATION_NVP( id );
          }
        };

        struct unregister_t
        {
          gpi::pc::type::segment_id_t id;

        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /*version*/)
          {
            ar & BOOST_SERIALIZATION_NVP( id );
          }
        };

        struct attach_t
        {
          gpi::pc::type::segment_id_t id; // id of an existing segment

        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /*version*/)
          {
            ar & BOOST_SERIALIZATION_NVP( id );
          }
        };

        struct detach_t
        {
          gpi::pc::type::segment_id_t id;

        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /*version*/)
          {
            ar & BOOST_SERIALIZATION_NVP( id );
          }
        };

        struct list_t
        {
          list_t (gpi::pc::type::segment_id_t seg_id = gpi::pc::type::segment::SEG_INVAL)
            : id (seg_id)
          {}

          gpi::pc::type::segment_id_t id;

        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /*version*/)
          {
            ar & BOOST_SERIALIZATION_NVP( id );
          }
        };

        struct list_reply_t
        {
          gpi::pc::type::segment::list_t list;
        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /*version*/)
          {
            ar & BOOST_SERIALIZATION_NVP( list );
          }
        };
      }
    }
  }
}

#endif

#ifndef GPI_SPACE_PC_PROTO_MEMORY_HPP
#define GPI_SPACE_PC_PROTO_MEMORY_HPP 1

#include <ostream>

#include <gpi-space/pc/type/typedefs.hpp>
#include <gpi-space/pc/type/handle_descriptor.hpp>

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
      namespace memory
      {
        struct alloc_t
        {
          gpi::pc::type::segment_id_t segment;
          gpi::pc::type::size_t size;
          gpi::pc::type::name_t name;
          gpi::pc::type::mode_t flags;

        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /*version*/)
          {
            ar & BOOST_SERIALIZATION_NVP( segment );
            ar & BOOST_SERIALIZATION_NVP( size );
            ar & BOOST_SERIALIZATION_NVP( name );
            ar & BOOST_SERIALIZATION_NVP( flags );
          }
        };

        inline
        std::ostream & operator<<(std::ostream & os, const alloc_t & a)
        {
          os << "ALLOC of " << a.size << " byte(s) in segment "
             << a.segment << " (" << a.name << ") with flags " << a.flags;
          return os;
        }

        struct alloc_reply_t
        {
          gpi::pc::type::handle_id_t handle;

        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /*version*/)
          {
            ar & BOOST_SERIALIZATION_NVP( handle );
          }
        };

        struct free_t
        {
          gpi::pc::type::handle_id_t handle;

        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /*version*/)
          {
            ar & BOOST_SERIALIZATION_NVP( handle );
          }
        };

        struct free_reply_t
        {
        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /*version*/)
          {
          }
        };

        struct list_t
        {
        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /*version*/)
          {
          }
        };

        struct list_reply_t
        {
          gpi::pc::type::handle::list_t list;

        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /*version*/)
          {
            ar & BOOST_SERIALIZATION_NVP( list );
          }
        };

        struct memory_location_t
        {
          gpi::pc::type::handle_id_t handle;
          gpi::pc::type::offset_t    offset;

        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /*version*/)
          {
            ar & BOOST_SERIALIZATION_NVP( handle );
            ar & BOOST_SERIALIZATION_NVP( offset );
          }
        };

        struct memcpy_t
        {
          memory_location_t dst;
          memory_location_t src;
          gpi::pc::type::size_t size;
          gpi::pc::type::queue_id_t queue;

        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /*version*/)
          {
            ar & BOOST_SERIALIZATION_NVP( dst );
            ar & BOOST_SERIALIZATION_NVP( src );
            ar & BOOST_SERIALIZATION_NVP( size );
            ar & BOOST_SERIALIZATION_NVP( queue );
          }
        };

        struct memcpy_reply_t
        {
          gpi::pc::type::queue_id_t queue;

        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /*version*/)
          {
            ar & BOOST_SERIALIZATION_NVP( queue );
          }
        };

        struct wait_t
        {
          gpi::pc::type::queue_id_t queue;

        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /*version*/)
          {
            ar & BOOST_SERIALIZATION_NVP( queue );
          }
        };

        struct wait_reply_t
        {
          gpi::pc::type::size_t count;

        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /*version*/)
          {
            ar & BOOST_SERIALIZATION_NVP( count );
          }
        };
      }
    }
  }
}

#endif

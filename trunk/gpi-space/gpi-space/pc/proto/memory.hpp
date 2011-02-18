#ifndef GPI_SPACE_PC_PROTO_MEMORY_HPP
#define GPI_SPACE_PC_PROTO_MEMORY_HPP 1

#include <ostream>

#include <boost/variant.hpp>

#include <gpi-space/pc/type/typedefs.hpp>
#include <gpi-space/pc/type/handle_descriptor.hpp>
#include <gpi-space/pc/type/memory_location.hpp>

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
          gpi::pc::type::flags_t flags;

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

        struct list_t
        {
          gpi::pc::type::segment_id_t segment;

        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /*version*/)
          {
            ar & BOOST_SERIALIZATION_NVP( segment );
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

        struct memcpy_t
        {
          gpi::pc::type::memory_location_t dst;
          gpi::pc::type::memory_location_t src;
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

        typedef boost::variant<
          memory::alloc_t
          , memory::alloc_reply_t
          , memory::free_t
          , memory::list_t
          , memory::list_reply_t
          , memory::memcpy_t
          , memory::memcpy_reply_t
          , memory::wait_t
          , memory::wait_reply_t
          > message_t;
      }
    }
  }
}

#endif

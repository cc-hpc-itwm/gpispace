#ifndef GPI_SPACE_PC_TYPE_HANDLE_DESCRIPTOR_HPP
#define GPI_SPACE_PC_TYPE_HANDLE_DESCRIPTOR_HPP 1

#include <vector>
#include <iostream>
#include <iomanip>

// serialization
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>

#include <gpi-space/pc/type/typedefs.hpp>
#include <gpi-space/pc/type/time_stamp.hpp>

namespace gpi
{
  namespace pc
  {
    namespace type
    {
      namespace handle
      {
        struct traits
        {
          static bool is_null (const gpi::pc::type::handle_id_t i)
          {
            return 0 == i;
          }
        };

        struct descriptor_t
        {
          gpi::pc::type::handle_id_t id;
          gpi::pc::type::segment_id_t segment;
          gpi::pc::type::offset_t offset;
          gpi::pc::type::size_t size;
          gpi::pc::type::name_t name;
          gpi::pc::type::mode_t flags;
          gpi::pc::type::time_stamp_t ts;

        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /*version*/)
          {
            ar & BOOST_SERIALIZATION_NVP( id );
            ar & BOOST_SERIALIZATION_NVP( segment );
            ar & BOOST_SERIALIZATION_NVP( offset );
            ar & BOOST_SERIALIZATION_NVP( size );
            ar & BOOST_SERIALIZATION_NVP( name );
            ar & BOOST_SERIALIZATION_NVP( flags );
            ar & BOOST_SERIALIZATION_NVP( ts );
          }
        };

        typedef std::vector<descriptor_t> list_t;

        inline
        std::ostream & operator<< (std::ostream & os, const descriptor_t & d)
        {
          std::ios_base::fmtflags saved_flags (os.flags());

          // ID
          os.flags (std::ios::right | std::ios::dec);
          os.width (5);
          os << d.id;
          os << " ";

          // SEGMENT
          os.flags (std::ios::right | std::ios::dec);
          os.width (5);
          os << d.segment;
          os << " ";

          // FLAGS
          os.flags (std::ios::right | std::ios::hex | std::ios::showbase);
          os.fill ('0');
          os.width (4);
          os << d.flags;
          os << " ";

          // OFFSET
          os.flags (std::ios::right | std::ios::dec);
          os.width (10);
          os << d.offset;
          os << " ";

          // SIZE
          os.flags (std::ios::right | std::ios::dec);
          os.width (12);
          os << d.size;
          os << " ";

          // CREATION TIME
          //    "%Y-%m-%d %H:%M"
          os.flags (std::ios::right | std::ios::dec);
          os.width (17);

          char buf[32];
          {
            struct tm tmp_tm;
            localtime_r (&d.ts.created, &tmp_tm);
            strftime (buf, sizeof(buf), "%Y-%m-%d %H:%M", &tmp_tm);
          }
          os << buf;
          os << " ";

          // NAME
          os.flags (std::ios::left);
          os << d.name;
          os << "  ";

          os.flags (saved_flags);

          return os;
        }

        inline
        std::ostream & operator<< (std::ostream & os, const list_t & list)
        {
          for ( list_t::const_iterator it (list.begin())
              ; it != list.end()
              ; ++it
              )
          {
            os << *it << std::endl;
          }

          return os;
        }
      }
    }
  }
}

#endif

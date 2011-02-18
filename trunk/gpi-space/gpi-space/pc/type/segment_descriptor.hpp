#ifndef GPI_SPACE_PC_TYPE_SEGMENT_DESCRIPTOR_HPP
#define GPI_SPACE_PC_TYPE_SEGMENT_DESCRIPTOR_HPP 1

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
      namespace segment
      {
        enum segment_type
          {
            SEG_INVAL = 0,
            SEG_GLOBAL,
            SEG_LOCAL,
            // the following is just a placeholder
            // shared segment ids are just >= SHARED
            SEG_SHARED,
          };

        enum flags_type
          {
            F_NONE         = 0x00,
            F_PERSISTENT   = 0x01, // leave segment in gpi after process death
            F_EXCLUSIVE    = 0x02, // no mapping possible from other processes
            F_NOUNLINK     = 0x04, // do not unlink segment after gpi termination
            F_NOCREATE     = 0x08, // do not create the segment (try to open it)
            F_FORCE_UNLINK = 0x10, // force recreation of the segment
          };

        struct traits
        {
          static bool is_valid (const gpi::pc::type::segment_id_t i)
          {
            return i != SEG_INVAL;
          }

          static bool is_global (const gpi::pc::type::segment_id_t i)
          {
            return i == SEG_GLOBAL;
          }
          static bool is_local (const gpi::pc::type::segment_id_t i)
          {
            return i == SEG_LOCAL;
          }
          static bool is_shared (const gpi::pc::type::segment_id_t i)
          {
            return i >= SEG_SHARED;
          }
        };

        struct descriptor_t
        {
          typedef traits traits_type;
          gpi::pc::type::segment_id_t id;
          gpi::pc::type::process_id_t creator;
          gpi::pc::type::name_t name;
          gpi::pc::type::size_t size;
          gpi::pc::type::ref_count_t nref;
          gpi::pc::type::flags_t flags;
          gpi::pc::type::time_stamp_t ts;

          descriptor_t ()
            : id (SEG_INVAL)
            , creator (0)
            , name ("")
            , size (0)
            , nref (0)
            , flags (0)
          {}

        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /*version*/)
          {
            ar & BOOST_SERIALIZATION_NVP( id );
            ar & BOOST_SERIALIZATION_NVP( creator );
            ar & BOOST_SERIALIZATION_NVP( name );
            ar & BOOST_SERIALIZATION_NVP( size );
            ar & BOOST_SERIALIZATION_NVP( nref );
            ar & BOOST_SERIALIZATION_NVP( flags );
            ar & BOOST_SERIALIZATION_NVP( ts );
          }
        };

        typedef std::vector<descriptor_t> list_t;

        inline
        std::ostream & operator << (std::ostream & os, const descriptor_t & d)
        {
          std::ios_base::fmtflags saved_flags (os.flags());
          char saved_fill = os.fill (' ');
          std::size_t saved_width = os.width (0);

          // ID
          os.flags (std::ios::right | std::ios::dec);
          os.width (5);
          os << d.id;
          os << " ";

          // REFCOUNT
          os.flags (std::ios::right | std::ios::dec);
          os.width (4);
          os << d.nref;
          os << " ";

          // FLAGS
          os.flags (std::ios::right | std::ios::hex);
          os.width (0);
          os << "0x";
          os.width (sizeof(gpi::pc::type::flags_t)*2);
          os.fill ('0');
          os << d.flags;
          os.fill (' ');
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
          os.fill (saved_fill);
          os.width (saved_width);

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

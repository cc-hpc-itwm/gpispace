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
            F_SPECIAL      = 0x20, // special segment (used internally to identify local/global segments)
            F_ATTACHED     = 0x40, // special flag indicating if the process container is attached
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
          gpi::pc::type::segment_id_t id;       // identification
          gpi::pc::type::process_id_t creator;  // container id
          gpi::pc::type::name_t name;           // user defined name
          gpi::pc::type::size_t size;           // maximum size
          gpi::pc::type::size_t avail;          // available size
          gpi::pc::type::size_t allocs;         // number of allocations
          gpi::pc::type::ref_count_t nref;      // number of containers attached
          gpi::pc::type::flags_t flags;         // flags (see above)
          gpi::pc::type::time_stamp_t ts;       // time stamps

          descriptor_t ()
            : id (SEG_INVAL)
            , creator (0)
            , name ("")
            , size (0)
            , avail (0)
            , allocs (0)
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
            ar & BOOST_SERIALIZATION_NVP( avail );
            ar & BOOST_SERIALIZATION_NVP( nref );
            ar & BOOST_SERIALIZATION_NVP( flags );
            ar & BOOST_SERIALIZATION_NVP( ts );
          }
        };

        typedef std::vector<descriptor_t> list_t;

        // just for ostream hacking
        struct segment_list_header {};

        inline
        std::ostream & operator << (std::ostream & os, const segment_list_header &)
        {
          std::ios_base::fmtflags saved_flags (os.flags());
          char saved_fill = os.fill (' ');
          std::size_t saved_width = os.width (0);

          os.flags (std::ios::adjustfield);

          std::cout << "  "; // special flag indicator
          // ID
          os.width (5);
          os << "ID";
          os << " ";

          // REFCOUNT
          os.width (4);
          os << "REF";
          os << " ";

          // ALLOCS
          os.width (8);
          os << "#ALLOC";
          os << " ";

          // FLAGS
          os.width (sizeof(gpi::pc::type::flags_t)*2 + 2);
          os << "FLAGS";
          os << " ";

          // SIZE
          os.width (12);
          os << "SIZE";
          os << " ";

          // CREATION TIME
          os.width (17);
          os << "TSTAMP";
          os << " ";

          // NAME
          os << "NAME";
          os << "  ";

          os.flags (saved_flags);
          os.fill (saved_fill);
          os.width (saved_width);

          return os;
        }

        inline
        std::ostream & operator << (std::ostream & os, const descriptor_t & d)
        {
          std::ios_base::fmtflags saved_flags (os.flags());
          char saved_fill = os.fill (' ');
          std::size_t saved_width = os.width (0);

          // visualize special flags
          if (gpi::flag::is_set (d.flags, F_SPECIAL))
          {
            os << "*";
          }
          else if (gpi::flag::is_set (d.flags, F_ATTACHED))
          {
            os << "+";
          }
          else
          {
            os << " ";
          }
          os << " ";

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

          // ALLOCS
          os.flags (std::ios::right | std::ios::dec);
          os.width (8);
          os << d.allocs;
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
          os << segment_list_header() << std::endl;

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

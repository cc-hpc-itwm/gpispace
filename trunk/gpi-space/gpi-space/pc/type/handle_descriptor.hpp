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
#include <gpi-space/pc/type/handle.hpp>
#include <gpi-space/pc/type/time_stamp.hpp>

namespace gpi
{
  namespace pc
  {
    namespace type
    {
      namespace handle
      {
        enum flags_type
          {
            F_NONE       = 0,
            F_PERSISTENT = 1 << 0,
            F_EXCLUSIVE  = 1 << 1,
            F_GLOBAL     = 1 << 2,
          };

        inline bool is_null (const gpi::pc::type::handle_id_t i)
        {
          return 0 == i;
        }

        struct descriptor_t
        {
          gpi::pc::type::handle_t id;
          gpi::pc::type::segment_id_t segment;
          gpi::pc::type::offset_t offset;
          gpi::pc::type::size_t size;
          gpi::pc::type::size_t nref;
          gpi::pc::type::name_t name;
          gpi::pc::type::process_id_t creator;
          gpi::pc::type::flags_t flags;
          gpi::pc::type::time_stamp_t ts;

          descriptor_t ()
            : id (0)
            , segment (0)
            , offset (0)
            , size (0)
            , nref (0)
            , name ("")
            , flags (0)
          {}

          bool operator< (const descriptor_t & other) const
          {
            return id < other.id;
          }

        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /*version*/)
          {
            ar & BOOST_SERIALIZATION_NVP( id.handle );
            ar & BOOST_SERIALIZATION_NVP( segment );
            ar & BOOST_SERIALIZATION_NVP( offset );
            ar & BOOST_SERIALIZATION_NVP( size );
            ar & BOOST_SERIALIZATION_NVP( nref );
            ar & BOOST_SERIALIZATION_NVP( name );
            ar & BOOST_SERIALIZATION_NVP( creator );
            ar & BOOST_SERIALIZATION_NVP( flags );
            ar & BOOST_SERIALIZATION_NVP( ts );
          }
        };

        typedef std::vector<descriptor_t> list_t;

        // just for ostream hacking
        struct ostream_header {};

        inline
        std::ostream & operator << (std::ostream & os, const ostream_header &)
        {
          std::ios_base::fmtflags saved_flags (os.flags());
          char saved_fill = os.fill (' ');
          std::size_t saved_width = os.width (0);

          os.flags (std::ios::adjustfield);

          // ID
          os.width (gpi::pc::type::handle_t::string_length);
          os << "ID";
          os << " ";

          // REFCOUNT
          os.width (4);
          os << "REF";
          os << " ";

          // SEGMENT
          os.width (5);
          os << "SEG";
          os << " ";

          // FLAGS
          os.width (sizeof(gpi::pc::type::flags_t)*2 + 2);
          os << "FLAGS";
          os << " ";

          // OFFSET
          os.width (12);
          os << "OFFSET";
          os << " ";

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
        std::ostream & operator<< (std::ostream & os, const descriptor_t & d)
        {
          std::ios_base::fmtflags saved_flags (os.flags());
          char saved_fill = os.fill (' ');
          std::size_t saved_width = os.width (0);

          // ID
          os.flags (std::ios::right | std::ios::dec);
          os << d.id;
          os << " ";

          // REFCOUNT
          os.flags (std::ios::right | std::ios::dec);
          os.width (4);
          os << d.nref;
          os << " ";

          // SEGMENT
          os.flags (std::ios::right | std::ios::dec);
          os.width (5);
          os << d.segment;
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

          // OFFSET
          os.flags (std::ios::right | std::ios::dec);
          os.width (12);
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

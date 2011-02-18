#ifndef GPI_SPACE_PC_TYPE_INFO_DESCRIPTOR_HPP
#define GPI_SPACE_PC_TYPE_INFO_DESCRIPTOR_HPP 1

#include <vector>
#include <iostream>
#include <iomanip>

// serialization
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>

#include <gpi-space/pc/type/typedefs.hpp>

namespace gpi
{
  namespace pc
  {
    namespace type
    {
      namespace info
      {
        struct descriptor_t
        {
          descriptor_t ()
            : rank (0)
            , nodes (0)
            , counters (0)
            , queues (0)
            , queue_depth (0)
          {}

          // gpi related information
          gpi::pc::type::size_t       rank;
          gpi::pc::type::size_t       nodes;
          gpi::pc::type::size_t       counters;
          gpi::pc::type::size_t       queues;
          gpi::pc::type::size_t       queue_depth;

        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /*version*/)
          {
            ar & BOOST_SERIALIZATION_NVP( rank );
            ar & BOOST_SERIALIZATION_NVP( nodes );
            ar & BOOST_SERIALIZATION_NVP( counters );
            ar & BOOST_SERIALIZATION_NVP( queues );
            ar & BOOST_SERIALIZATION_NVP( queue_depth );
          }
        };

        inline
        std::ostream & operator<< (std::ostream & os, const descriptor_t & d)
        {
          std::ios_base::fmtflags saved_flags (os.flags());
          char saved_fill = os.fill (' ');
          std::size_t saved_width = os.width (0);

          std::cout << "rank="  << d.rank << " ";
          std::cout << "nodes=" << d.nodes << " ";
          std::cout << "counters=" << d.counters << " ";
          std::cout << "queues="   << d.queues << " ";
          std::cout << "depth="   << d.queue_depth;

          os.flags (saved_flags);
          os.fill (saved_fill);
          os.width (saved_width);

          return os;
        }
      }
    }
  }
}

#endif

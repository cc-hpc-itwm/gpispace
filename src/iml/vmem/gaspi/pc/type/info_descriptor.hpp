#pragma once

#include <vector>

// serialization
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>

#include <iml/vmem/gaspi/pc/type/types.hpp>

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
            , queues (0)
            , queue_depth (0)
          {}

          // gpi related information
          gpi::pc::type::size_t       rank;
          gpi::pc::type::size_t       nodes;
          gpi::pc::type::size_t       queues;
          gpi::pc::type::size_t       queue_depth;

        private:
          friend class boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, const unsigned int /*version*/)
          {
            ar & BOOST_SERIALIZATION_NVP( rank );
            ar & BOOST_SERIALIZATION_NVP( nodes );
            ar & BOOST_SERIALIZATION_NVP( queues );
            ar & BOOST_SERIALIZATION_NVP( queue_depth );
          }
        };
      }
    }
  }
}

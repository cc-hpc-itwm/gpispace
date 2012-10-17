/**
 * @file   itopology.hpp
 * @author Alexander Petry <petry@itwm.fhg.de>
 *
 * @brief  Topology interface for global operations
 */

#ifndef GPI_SPACE_GLOBAL_ITOPOLOGY_HPP
#define GPI_SPACE_GLOBAL_ITOPOLOGY_HPP 1

#include <gpi-space/types.hpp>
#include <gpi-space/pc/type/typedefs.hpp>
#include <gpi-space/pc/type/handle.hpp>

namespace gpi
{
  namespace pc
  {
    namespace global
    {
      class itopology_t
      {
      public:
        virtual ~itopology_t () {}

        virtual bool is_master () const = 0;

        // initiate a global alloc
        virtual int alloc ( const gpi::pc::type::segment_id_t segment
                          , const gpi::pc::type::handle_t
                          , const gpi::pc::type::offset_t
                          , const gpi::pc::type::size_t size
                          , const gpi::pc::type::size_t local_size
                          , const std::string & name
                          ) = 0;

        virtual int free (const gpi::pc::type::handle_t) = 0;

        virtual int add_memory ( const gpi::pc::type::segment_id_t seg_id
                               , std::string const & url
                               ) = 0;

        virtual int del_memory (const gpi::pc::type::segment_id_t seg_id) = 0;
      };
    }
  }
}

#endif // GPI_SPACE_GLOBAL_ITOPOLOGY_HPP

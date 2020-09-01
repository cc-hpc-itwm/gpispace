#pragma once

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
        virtual ~itopology_t () = default;

        virtual bool is_master () const = 0;

        // initiate a global alloc
        virtual void alloc ( const gpi::pc::type::segment_id_t segment
                           , const gpi::pc::type::handle_t
                           , const gpi::pc::type::offset_t
                           , const gpi::pc::type::size_t size
                           , const gpi::pc::type::size_t local_size
                           , const std::string & name
                           ) = 0;

        virtual void free (const gpi::pc::type::handle_t) = 0;

        virtual void add_memory ( const gpi::pc::type::segment_id_t seg_id
                                , std::string const & url
                                ) = 0;

        virtual void del_memory (const gpi::pc::type::segment_id_t seg_id) = 0;
      };
    }
  }
}

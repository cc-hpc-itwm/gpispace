#pragma once

#include <gpi-space/pc/global/itopology.hpp>

namespace gpi
{
  namespace tests
  {
    class dummy_topology : public gpi::pc::global::itopology_t
    {
    public:
      virtual void alloc ( const gpi::pc::type::segment_id_t /* segment */
                , const gpi::pc::type::handle_t /* handle */
                , const gpi::pc::type::offset_t /* offset */
                , const gpi::pc::type::size_t /* size */
                , const gpi::pc::type::size_t /* local_size */
                , const std::string & /* name */
                ) override
      {}

      virtual void free (const gpi::pc::type::handle_t) override
      {}

      virtual void add_memory ( const gpi::pc::type::segment_id_t
                     , const std::string &
                     ) override
      {}

      virtual void del_memory (const gpi::pc::type::segment_id_t) override
      {}

      virtual bool is_master () const override
      {
        return true;
      }
    };
  }
}

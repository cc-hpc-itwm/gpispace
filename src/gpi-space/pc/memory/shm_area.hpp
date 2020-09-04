// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <gpi-space/pc/type/segment_type.hpp>
#include <gpi-space/pc/memory/memory_area.hpp>

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      class shm_area_t : public virtual area_t
      {
      public:
        static const type::segment::segment_type area_type = gpi::pc::type::segment::SEG_SHM;

        shm_area_t ( fhg::logging::stream_emitter&
                   , const gpi::pc::type::process_id_t creator
                   , type::name_t const&
                   , const gpi::pc::type::size_t size
                   , handle_generator_t&
                   );

        ~shm_area_t ();
      protected:
        virtual gspc::vmem::dtmmgr::Arena_t grow_direction (const gpi::pc::type::flags_t) const override;
        int get_type_id () const;

        virtual void alloc_hook (const gpi::pc::type::handle::descriptor_t &) override{}
        virtual void  free_hook (const gpi::pc::type::handle::descriptor_t &) override{}

      private:
        virtual void *raw_ptr (gpi::pc::type::offset_t off) override;

        virtual bool is_range_local ( const gpi::pc::type::handle::descriptor_t &
                            , const gpi::pc::type::offset_t a
                            , const gpi::pc::type::offset_t b
                            ) const override;
        virtual gpi::pc::type::size_t get_local_size ( const gpi::pc::type::size_t size
                                             , const gpi::pc::type::flags_t flgs
                                             ) const override;


        double get_transfer_costs ( const gpi::pc::type::memory_region_t&
                                  , const gpi::rank_t
                                  ) const override;

        void *m_ptr;
        std::string m_path;
      };
    }
  }
}

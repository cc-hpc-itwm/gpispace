// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
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

#include <iml/MemoryOffset.hpp>
#include <iml/MemorySize.hpp>
#include <iml/vmem/gaspi/pc/memory/memory_area.hpp>

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      class shm_area_t : public virtual area_t
      {
      public:
        shm_area_t ( type::name_t const&
                   , iml::MemorySize size
                   );

        virtual ~shm_area_t () override;
      protected:
        virtual bool is_shm_segment() const override;

        virtual global::itopology_t& global_topology() override;

      private:
        virtual void *raw_ptr (iml::MemoryOffset off) override;

        virtual bool is_range_local ( gpi::pc::type::handle::descriptor_t const&
                            , iml::MemoryOffset a
                            , iml::MemoryOffset b
                            ) const override;
        virtual iml::MemorySize get_local_size ( iml::MemorySize size
                                             , gpi::pc::type::flags_t flgs
                                             ) const override;


        double get_transfer_costs ( iml::MemoryRegion const&
                                  , gpi::rank_t
                                  ) const override;

        void *m_ptr;
        std::string m_path;
        std::size_t _size;
      };
    }
  }
}

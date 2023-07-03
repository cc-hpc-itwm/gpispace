// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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

        ~shm_area_t () override;
        shm_area_t (shm_area_t const&) = delete;
        shm_area_t (shm_area_t&&) = delete;
        shm_area_t& operator= (shm_area_t const&) = delete;
        shm_area_t& operator= (shm_area_t&&) = delete;

      protected:
        bool is_shm_segment() const override;

        global::itopology_t& global_topology() override;

      private:
        void *raw_ptr (iml::MemoryOffset off) override;

        bool is_range_local ( gpi::pc::type::handle::descriptor_t const&
                            , iml::MemoryOffset a
                            , iml::MemoryOffset b
                            ) const override;
        iml::MemorySize get_local_size ( iml::MemorySize size
                                             , gpi::pc::type::flags_t flgs
                                             ) const override;


        double get_transfer_costs ( iml::MemoryRegion const&
                                  , gpi::rank_t
                                  ) const override;

        void *m_ptr {nullptr};
        std::string m_path;
        std::size_t _size;
      };
    }
  }
}

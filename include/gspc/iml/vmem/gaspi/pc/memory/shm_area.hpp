// Copyright (C) 2011-2012,2014-2015,2020-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/iml/MemoryOffset.hpp>
#include <gspc/iml/MemorySize.hpp>
#include <gspc/iml/vmem/gaspi/pc/memory/memory_area.hpp>



    namespace gpi::pc::memory
    {
      class shm_area_t : public virtual area_t
      {
      public:
        shm_area_t ( type::name_t const&
                   , gspc::iml::MemorySize size
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
        void *raw_ptr (gspc::iml::MemoryOffset off) override;

        bool is_range_local ( gpi::pc::type::handle::descriptor_t const&
                            , gspc::iml::MemoryOffset a
                            , gspc::iml::MemoryOffset b
                            ) const override;
        gspc::iml::MemorySize get_local_size ( gspc::iml::MemorySize size
                                             , gpi::pc::type::flags_t flgs
                                             ) const override;


        double get_transfer_costs ( gspc::iml::MemoryRegion const&
                                  , gpi::rank_t
                                  ) const override;

        void *m_ptr {nullptr};
        std::string m_path;
        std::size_t _size;
      };
    }

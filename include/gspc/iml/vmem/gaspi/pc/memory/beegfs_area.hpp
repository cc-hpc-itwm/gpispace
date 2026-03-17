// Copyright (C) 2012,2014-2015,2020-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/iml/MemoryOffset.hpp>
#include <gspc/iml/MemorySize.hpp>
#include <gspc/iml/beegfs/SegmentDescription.hpp>
#include <gspc/iml/vmem/gaspi/pc/global/itopology.hpp>
#include <gspc/iml/vmem/gaspi/pc/memory/memory_area.hpp>

#include <gspc/util/filesystem_lock_directory.hpp>
#include <gspc/util/threadsafe_queue.hpp>

#include <filesystem>
#include <optional>



    namespace gpi::pc::memory
    {
      class beegfs_area_t : public area_t
      {
      public:
        using path_t = std::filesystem::path;

        static const int BEEGFS_AREA_VERSION = 0x0001;

        static area_ptr_t create ( gspc::iml::beegfs::SegmentDescription const&
                                 , unsigned long total_size
                                 , gpi::pc::global::itopology_t & topology
                                 , bool is_creator
                                 );

        beegfs_area_t ( bool is_creator
                      , path_t const& path
                      , gspc::iml::MemorySize size        // total
                      , gpi::pc::global::itopology_t & topology
                      );

        ~beegfs_area_t () override;
        beegfs_area_t (beegfs_area_t const&) = delete;
        beegfs_area_t (beegfs_area_t&&) = delete;
        beegfs_area_t& operator= (beegfs_area_t const&) = delete;
        beegfs_area_t& operator= (beegfs_area_t&&) = delete;

      protected:
        global::itopology_t& global_topology() override;

      private:
        bool is_range_local ( gpi::pc::type::handle::descriptor_t const&
                            , gspc::iml::MemoryOffset begin
                            , gspc::iml::MemorySize   range_size
                            ) const override;

        void *raw_ptr (gspc::iml::MemoryOffset off) override;

        gspc::iml::MemorySize get_local_size ( gspc::iml::MemorySize size
                                             , gpi::pc::type::flags_t flags
                                             ) const override;

        gspc::iml::MemorySize
        read_from_impl ( gspc::iml::MemoryOffset off
                       , void *buffer
                       , gspc::iml::MemorySize amount
                       ) override;

        gspc::iml::MemorySize
        write_to_impl ( gspc::iml::MemoryOffset off
                      , const void *buffer
                      , gspc::iml::MemorySize amount
                      ) override;

        std::string version_string() const;

        void open (std::size_t total_size);
        void close();
        double get_transfer_costs ( gspc::iml::MemoryRegion const&
                                  , gpi::rank_t
                                  ) const override;

        gspc::util::threadsafe_queue<int> _fds;
        struct lock_file_helper : gspc::util::filesystem_lock_directory
        {
          lock_file_helper (beegfs_area_t&);
        };
        bool _is_creator;
        std::optional<lock_file_helper> _lock_file;
        path_t m_path;
        int    m_version;

        gpi::pc::global::itopology_t & m_topology;
      };
    }

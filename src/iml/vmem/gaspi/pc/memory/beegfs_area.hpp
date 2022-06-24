// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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

#include <boost/filesystem.hpp>

#include <iml/MemoryOffset.hpp>
#include <iml/MemorySize.hpp>
#include <iml/beegfs/SegmentDescription.hpp>
#include <iml/vmem/gaspi/pc/global/itopology.hpp>
#include <iml/vmem/gaspi/pc/memory/memory_area.hpp>

#include <util-generic/filesystem_lock_directory.hpp>
#include <util-generic/threadsafe_queue.hpp>

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      class beegfs_area_t : public area_t
      {
      public:
        using path_t = ::boost::filesystem::path;

        static const int BEEGFS_AREA_VERSION = 0x0001;

        static area_ptr_t create ( iml::beegfs::SegmentDescription const&
                                 , unsigned long total_size
                                 , gpi::pc::global::itopology_t & topology
                                 , bool is_creator
                                 );

        beegfs_area_t ( bool is_creator
                      , path_t const& path
                      , iml::MemorySize size        // total
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
                            , iml::MemoryOffset begin
                            , iml::MemorySize   range_size
                            ) const override;

        void *raw_ptr (iml::MemoryOffset off) override;

        iml::MemorySize get_local_size ( iml::MemorySize size
                                             , gpi::pc::type::flags_t flags
                                             ) const override;

        iml::MemorySize
        read_from_impl ( iml::MemoryOffset off
                       , void *buffer
                       , iml::MemorySize amount
                       ) override;

        iml::MemorySize
        write_to_impl ( iml::MemoryOffset off
                      , const void *buffer
                      , iml::MemorySize amount
                      ) override;

        std::string version_string() const;

        void open (std::size_t total_size);
        void close();
        double get_transfer_costs ( iml::MemoryRegion const&
                                  , gpi::rank_t
                                  ) const override;

        fhg::util::threadsafe_queue<int> _fds;
        struct lock_file_helper : fhg::util::filesystem_lock_directory
        {
          lock_file_helper (beegfs_area_t&);
        };
        bool _is_creator;
        //! \todo ::boost::optional with move-assignment support
        std::unique_ptr<lock_file_helper> _lock_file;
        path_t m_path;
        int    m_version;

        gpi::pc::global::itopology_t & m_topology;
      };
    }
  }
}

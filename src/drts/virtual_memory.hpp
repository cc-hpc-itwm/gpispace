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

#include <drts/virtual_memory.fwd.hpp>
#include <drts/pimpl.hpp>
#include <drts/drts.fwd.hpp>
#include <drts/stream.fwd.hpp>

#include <we/type/value.hpp>

#include <boost/filesystem/path.hpp>

#include <string>

namespace gspc
{
  class scoped_runtime_system;

  namespace vmem
  {
    struct gaspi_segment_description
    {
      inline gaspi_segment_description
        ( std::size_t communication_buffer_size = 4 * (1 << 20)
        , std::size_t communication_buffer_count = 8
        );

      std::size_t _communication_buffer_size;
      std::size_t _communication_buffer_count;
    };
    struct beegfs_segment_description
    {
      inline beegfs_segment_description (boost::filesystem::path);

      boost::filesystem::path _path;
    };
  }

  class vmem_allocation
  {
  private:
    friend class scoped_runtime_system;
    friend class stream;

    vmem_allocation ( scoped_runtime_system const* const
                    , vmem::segment_description
                    , unsigned long size
                    , std::string const& description
                    );
    vmem_allocation ( scoped_runtime_system const* const
                    , vmem::segment_description
                    , unsigned long size
                    , std::string const& description
                    , char const* const datia
                    );

  public:
    std::size_t size() const;

    pnet::type::value::value_type global_memory_range() const;
    pnet::type::value::value_type global_memory_range ( std::size_t const offset
                                                      , std::size_t const size
                                                      ) const;

    vmem_allocation (vmem_allocation const&) = delete;
    vmem_allocation& operator= (vmem_allocation const&) = delete;

    vmem_allocation (vmem_allocation&&);
    vmem_allocation& operator= (vmem_allocation&&) = delete;

    PIMPL (vmem_allocation);
  };

  vmem::gaspi_segment_description::gaspi_segment_description
      ( std::size_t communication_buffer_size
      , std::size_t communication_buffer_count
      )
    : _communication_buffer_size (communication_buffer_size)
    , _communication_buffer_count (communication_buffer_count)
  {}

  vmem::beegfs_segment_description::beegfs_segment_description
      (boost::filesystem::path path)
    : _path (std::move (path))
  {}
}

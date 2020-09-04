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

#include <gpi-space/pc/client/api.hpp>
#include <gpi-space/pc/segment/segment.hpp>
#include <gpi-space/pc/type/handle.hpp>

namespace gspc
{
  class scoped_allocation
  {
    class scoped_segment
    {
    public:
      scoped_segment
        ( std::unique_ptr<gpi::pc::client::api_t> const& virtual_memory
        , std::string const& description
        , unsigned long size
        )
          : _virtual_memory (virtual_memory)
          , _segment (_virtual_memory->register_segment (description, size))
      {}

      ~scoped_segment()
      {
        _virtual_memory->unregister_segment (_segment);
      }

      operator gpi::pc::type::handle_id_t const& () const
      {
        return _segment;
      }

    private:
      std::unique_ptr<gpi::pc::client::api_t> const& _virtual_memory;
      gpi::pc::type::handle_id_t const _segment;
    };

  public:
    scoped_allocation
      ( std::unique_ptr<gpi::pc::client::api_t> const& virtual_memory
      , std::string const& description
      , unsigned long size
      )
        : _virtual_memory (virtual_memory)
        , _scoped_segment (scoped_segment (_virtual_memory, description, size))
        , _handle (_virtual_memory->alloc
                    ( _scoped_segment
                    , size
                    , description
                    , gpi::pc::F_EXCLUSIVE
                    )
                  )
        , _size (size)
    {}

    ~scoped_allocation()
    {
      _virtual_memory->free (_handle);
    }

    operator gpi::pc::type::handle_t const& () const
    {
      return _handle;
    }

    unsigned long size() const
    {
      return _size;
    }

  private:
    std::unique_ptr<gpi::pc::client::api_t> const& _virtual_memory;
    scoped_segment const _scoped_segment;
    gpi::pc::type::handle_t const _handle;
    unsigned long const _size;
  };
}

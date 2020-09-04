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

#include <unistd.h> // size_t

#include <gpi-space/pc/type/handle.hpp>

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      class handle_buffer_t
      {
      public:
        explicit
        handle_buffer_t ( gpi::pc::type::handle_t hdl
                        , size_t sz
                        , void *ptr
                        )
          : m_handle (hdl)
          , m_data (ptr)
          , m_size (sz)
          , m_used (0)
        {}

        inline void *data ()        { return m_data; }
        inline size_t size () const { return m_size; }
        inline size_t used () const { return m_used; }
        inline void used (size_t u) { m_used = u; }

        gpi::pc::type::handle_t handle () const { return m_handle; }
      private:
        gpi::pc::type::handle_t  m_handle;
        void                    *m_data;
        size_t                   m_size;
        size_t                   m_used;
      };
    }
  }
}

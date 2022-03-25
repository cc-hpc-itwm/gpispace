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

#include <iml/MemorySize.hpp>

#include <cstddef>
#include <string>

namespace iml
{
  namespace detail
  {
    class OpenedSharedMemory
    {
    public:
      OpenedSharedMemory ( std::string const& name
                         , iml::MemorySize sz
                         );

      ~OpenedSharedMemory();
      OpenedSharedMemory (OpenedSharedMemory const&) = delete;
      OpenedSharedMemory (OpenedSharedMemory&&) = delete;
      OpenedSharedMemory& operator= (OpenedSharedMemory const&) = delete;
      OpenedSharedMemory& operator= (OpenedSharedMemory&&) = delete;

      void create (mode_t mode = 00600);
      void open ();
      void close ();
      void unlink ();

      template<typename T>
        T* ptr () { return static_cast<T*> (ptr()); }

      void *ptr ();
      const void *ptr () const;
    private:
      void *m_ptr;
      std::string _name;
      std::size_t _size;
    };
  }
}

namespace gpi
{
  namespace pc
  {
    namespace segment
    {
      using segment_t = iml::detail::OpenedSharedMemory;
    }
  }
}

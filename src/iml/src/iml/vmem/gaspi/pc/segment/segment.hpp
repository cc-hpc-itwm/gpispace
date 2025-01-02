// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
      void *m_ptr {nullptr};
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

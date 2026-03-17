// Copyright (C) 2011,2015,2020-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/iml/MemorySize.hpp>

#include <cstddef>
#include <string>


  namespace gspc::iml::detail
  {
    class OpenedSharedMemory
    {
    public:
      OpenedSharedMemory ( std::string const& name
                         , gspc::iml::MemorySize sz
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




    namespace gpi::pc::segment
    {
      using segment_t = gspc::iml::detail::OpenedSharedMemory;
    }

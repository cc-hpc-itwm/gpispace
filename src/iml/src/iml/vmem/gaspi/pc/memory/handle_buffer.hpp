// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <iml/AllocationHandle.hpp>

#include <cstddef>

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
        handle_buffer_t ( iml::AllocationHandle hdl
                        , std::size_t sz
                        , void *ptr
                        )
          : m_handle (hdl)
          , m_data (ptr)
          , m_size (sz)
        {}

        inline void *data ()        { return m_data; }
        inline std::size_t size () const { return m_size; }
        inline std::size_t used () const { return m_used; }
        inline void used (std::size_t u) { m_used = u; }

        iml::AllocationHandle handle () const { return m_handle; }
      private:
        iml::AllocationHandle  m_handle;
        void                    *m_data;
        std::size_t                   m_size;
        std::size_t                   m_used {0};
      };
    }
  }
}

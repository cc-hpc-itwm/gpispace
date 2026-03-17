// Copyright (C) 2014,2021-2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/detail/export.hpp>

#include <gspc/we/type/bytearray.hpp>

#include <utility>

namespace map
{
  using user_data_type = gspc::we::type::bytearray;
  using size_in_bytes_type = unsigned long;
  using memory_buffer_type = std::pair<void*, size_in_bytes_type>;
  using const_memory_buffer_type = std::pair<const void*, size_in_bytes_type>;
}

extern "C"
{
  GSPC_EXPORT void map_produce ( map::user_data_type const&
                                  , map::memory_buffer_type
                                  , unsigned long id
                                  );

  GSPC_EXPORT void map_process ( map::user_data_type const&
                                  , map::const_memory_buffer_type
                                  , map::memory_buffer_type
                                  );

  GSPC_EXPORT void map_consume ( map::user_data_type const&
                                  , map::const_memory_buffer_type
                                  , unsigned long id
                                  );
}

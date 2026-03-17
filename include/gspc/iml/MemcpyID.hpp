// Copyright (C) 2020,2022-2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>

namespace gspc::iml
{
  //! An identifier for an ongoing memory transfer.
  //! \see Client::async_memcpy(), Client::wait()
  using MemcpyID = std::uint64_t;
}

// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#if GSPC_WITH_IML
  #include <iml/SharedMemoryAllocation.hpp>
#else
namespace iml
{
  class SharedMemoryAllocation {};
}
#endif

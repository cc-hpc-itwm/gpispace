// Copyright (C) 2023-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/util/temporary_path.hpp>
#include <gspc/testing/unique_path.hpp>

namespace gspc::testing
{
  class temporary_path : public ::gspc::util::temporary_path
  {
  public:
    using ::gspc::util::temporary_path::temporary_path;

    temporary_path()
      : ::gspc::util::temporary_path { ::gspc::testing::unique_path() }
    {}
  };
}

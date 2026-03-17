// Copyright (C) 2010,2013-2015,2021-2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <atomic>
#include <string>

namespace gspc::scheduler
{
  class id_generator
  {
  public:
    std::string next();

    id_generator (std::string const& name);

  private:
    std::atomic<std::size_t> _counter;
    std::string _prefix;
  };
}

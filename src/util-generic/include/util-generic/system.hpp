// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>

namespace fhg
{
  namespace util
  {
    using Description = std::string;
    using Command = std::string;

    template<typename STRable>
      void system (STRable);

    template<>
      void system<Command> (Command);

    template<typename Exception, typename Command>
      void system (Description, Command);
  }
}

#include <util-generic/system.ipp>

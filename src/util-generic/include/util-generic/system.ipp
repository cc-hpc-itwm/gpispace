// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/nest_exceptions.hpp>

namespace fhg
{
  namespace util
  {
    template<typename STRable>
      void system (STRable command)
    {
      return system (command.str());
    }

    template<typename Exception, typename Command>
      void system (Description description, Command command)
    {
      return nest_exceptions<Exception>
        ( [&command]
          {
            system (command);
          }
        , "Could not " + description
        );
    }
  }
}

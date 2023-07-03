// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <we/test/eureka/jobserver/EurekaGroup.hpp>
#include <we/test/eureka/jobserver/Task.hpp>

#include <memory>
#include <string>

namespace gspc
{
  namespace we
  {
    namespace test
    {
      namespace eureka
      {
        namespace jobserver
        {
          struct client
          {
            client ( std::string host
                   , unsigned short port
                   , Task task
                   , EurekaGroup eureka_group
                   );
            void cancelled();
            void exited_or_cancelled();
            ~client();
            client (client const&) = delete;
            client (client&&) = delete;
            client& operator= (client const&) = delete;
            client& operator= (client&&) = delete;

          private:
            struct implementation;
            std::unique_ptr<implementation> _implementation;
          };
        }
      }
    }
  }
}

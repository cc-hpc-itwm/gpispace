// Copyright (C) 2021-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <test/we/eureka/jobserver/EurekaGroup.hpp>
#include <test/we/eureka/jobserver/Task.hpp>

#include <memory>
#include <string>





        namespace gspc::we::test::eureka::jobserver
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

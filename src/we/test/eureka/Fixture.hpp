// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <we/test/eureka/jobserver/provider.hpp>

#include <util-generic/testing/random.hpp>

#include <string>
#include <vector>

namespace gspc
{
  namespace we
  {
    namespace test
    {
      namespace eureka
      {
        struct Fixture
        {
          using JobServer = jobserver::JobServer;
          using EurekaGroup = jobserver::EurekaGroup;
          using Task = jobserver::Task;
          using TasksByEurekaGroup = jobserver::TasksByEurekaGroup;

          JobServer jobserver;

          struct random
          {
            unsigned int sleep_time();
            unsigned int bit();

            int eureka_value();
            Task unique_task();
            EurekaGroup unique_eureka_group();

            std::vector<int> eureka_values (unsigned int);
            std::vector<Task> unique_tasks (unsigned int);
            std::vector<EurekaGroup> unique_eureka_groups (unsigned int);

            unsigned int between (unsigned int min, unsigned int max);

          private:
            fhg::util::testing::random<int> _int;
            fhg::util::testing::random<unsigned int> _uint;

            struct identifier
            {
              std::string operator()() const;
            };
            fhg::util::testing::unique_random<EurekaGroup, identifier>
              _unique_eureka_group;
            fhg::util::testing::unique_random<Task, identifier>
              _unique_task;
          };

          struct random random;
        };
      }
    }
  }
}

#define WE_TEST_EUREKA_PARSE_COMMAND_LINE_COMPILE_PNET_BOOT_AND_SUBMIT(workflow,topology) \
  WE_TEST_EUREKA_PARSE_COMMAND_LINE_COMPILE_PNET_BOOT_AND_SUBMIT_IMPL (workflow, topology)

#define WE_TEST_EUREKA_PUT_TOKEN(place,value) \
  WE_TEST_EUREKA_PUT_TOKEN_IMPL (place, value)

#define WE_TEST_EUREKA_GET_RESULT() \
  WE_TEST_EUREKA_GET_RESULT_IMPL()

#define NUMBER_OF_NODES() \
  NUMBER_OF_NODES_IMPL()

#include <we/test/eureka/Fixture.ipp>

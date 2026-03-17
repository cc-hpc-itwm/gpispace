// Copyright (C) 2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <test/we/eureka/jobserver/provider.hpp>

#include <gspc/testing/random.hpp>

#include <string>
#include <vector>




      namespace gspc::we::test::eureka
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
            gspc::testing::random<int> _int;
            gspc::testing::random<unsigned int> _uint;

            struct identifier
            {
              std::string operator()() const;
            };
            gspc::testing::unique_random<EurekaGroup, identifier>
              _unique_eureka_group;
            gspc::testing::unique_random<Task, identifier>
              _unique_task;
          };

          struct random random;
        };
      }




#define WE_TEST_EUREKA_PARSE_COMMAND_LINE_COMPILE_PNET_BOOT_AND_SUBMIT(workflow,topology) \
  WE_TEST_EUREKA_PARSE_COMMAND_LINE_COMPILE_PNET_BOOT_AND_SUBMIT_IMPL (workflow, topology)

#define WE_TEST_EUREKA_PUT_TOKEN(place,value) \
  WE_TEST_EUREKA_PUT_TOKEN_IMPL (place, value)

#define WE_TEST_EUREKA_GET_RESULT() \
  WE_TEST_EUREKA_GET_RESULT_IMPL()

#define NUMBER_OF_NODES() \
  NUMBER_OF_NODES_IMPL()

#include <test/we/eureka/Fixture.ipp>

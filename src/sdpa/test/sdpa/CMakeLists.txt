# This file is part of GPI-Space.
# Copyright (C) 2021 Fraunhofer ITWM
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.

if (BUILD_TESTING)
  gspc_add_library (NAME SDPATestUtilities
    TYPE STATIC
    NAMESPACE GPISpace
    SOURCES "utils.cpp"
    LIBRARIES sdpa
  )
endif()

fhg_add_test (NAME sdpa_AddNewWorkers
  SOURCES AddNewWorkers.cpp
  USE_BOOST
  LIBRARIES GPISpace::SDPATestUtilities
            sdpa
            test-utilities
)

fhg_add_test (NAME sdpa_CancelJob
  SOURCES CancelJob.cpp
  USE_BOOST
  LIBRARIES GPISpace::SDPATestUtilities
            sdpa
            test-utilities
)

fhg_add_test (NAME sdpa_Coallocation
  SOURCES Coallocation.cpp
  USE_BOOST
  LIBRARIES GPISpace::SDPATestUtilities
            sdpa
            test-utilities
)

fhg_add_test (NAME sdpa_CoallocationScheduler
  SOURCES CoallocationScheduler.cpp
  USE_BOOST
  LIBRARIES GPISpace::SDPATestUtilities
            sdpa
            test-utilities
)

fhg_add_test (NAME sdpa_daemon_message_flow
  SOURCES daemon_message_flow.cpp
  DESCRIPTION "Message flow between client/agent and children/parents"
  USE_BOOST
  LIBRARIES GPISpace::SDPATestUtilities
            sdpa
            test-utilities
)

fhg_add_test (NAME sdpa_InvalidNbWorkersReq
  SOURCES InvalidNbWorkersReq.cpp
  USE_BOOST
  LIBRARIES GPISpace::SDPATestUtilities
            sdpa
            test-utilities
)

fhg_add_test (NAME sdpa_RestartWorkersCoalloc
  SOURCES RestartWorkersCoalloc.cpp
  NAME restart_workers_with_coallocation_workflow
  USE_BOOST
  LIBRARIES GPISpace::SDPATestUtilities
            sdpa
            test-utilities
)

fhg_add_test (NAME sdpa_RestartWorkersDummy
  SOURCES RestartWorkersDummy.cpp
  NAME restart_worker_with_dummy_workflow
  USE_BOOST
  LIBRARIES GPISpace::SDPATestUtilities
            sdpa
            test-utilities
)

fhg_add_test (NAME sdpa_Scheduler_no_preferences.performance
  SOURCES Scheduler_no_preferences.performance.cpp
  USE_BOOST
  PERFORMANCE_TEST
  RUN_SERIAL
  LIBRARIES GPISpace::SDPATestUtilities
            sdpa
            test-utilities
)

fhg_add_test (NAME sdpa_Scheduler_with_preferences.performance
  SOURCES Scheduler_with_preferences.performance.cpp
  USE_BOOST
  PERFORMANCE_TEST
  RUN_SERIAL
  LIBRARIES GPISpace::SDPATestUtilities
            sdpa
            test-utilities
)

fhg_add_test (NAME sdpa_Subscriber
  SOURCES Subscriber.cpp
  USE_BOOST
  LIBRARIES GPISpace::SDPATestUtilities
            sdpa
            test-utilities
)

fhg_add_test (NAME sdpa_WorkerManager
  SOURCES WorkerManager.cpp
  USE_BOOST
  LIBRARIES GPISpace::SDPATestUtilities
            sdpa
            test-utilities
)

fhg_add_test (NAME remove_workers_in_a_coallocation_job_and_add_them_again
  SOURCES remove_workers_in_a_coallocation_job.cpp
  USE_BOOST
  LIBRARIES GPISpace::SDPATestUtilities
            sdpa
            test-utilities
)

fhg_add_test (NAME forbid_equal_preferences_for_tasks
  SOURCES forbid_equal_preferences_for_tasks.cpp
  USE_BOOST
  LIBRARIES sdpa
            test-utilities
)

fhg_add_test (NAME sdpa_Preferences
  SOURCES Preferences.cpp
  USE_BOOST
  LIBRARIES GPISpace::SDPATestUtilities
            sdpa
            test-utilities
)

fhg_add_test (NAME sdpa_network_basics
  SOURCES network_basics.cpp
  USE_BOOST
  LIBRARIES GPISpace::SDPATestUtilities
            sdpa
            test-utilities
)

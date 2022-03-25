# This file is part of GPI-Space.
# Copyright (C) 2022 Fraunhofer ITWM
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

gspc_add_library (NAME share_example_parallel_reduce_module_call_Task
  NAMESPACE GSPC
  TYPE STATIC
  SOURCES "Task.cpp"
  LIBRARIES
    PRIVATE Util::Generic
    PRIVATE Boost::serialization
  INCLUDE_DIRECTORIES PRIVATE "${PROJECT_SOURCE_DIR}/"
)

gspc_add_library (NAME share_example_parallel_reduce_module_call_JobExecutor
  NAMESPACE GSPC
  TYPE STATIC
  SOURCES "JobExecutor.cpp"
  LIBRARIES
    PRIVATE Util::RPC
    PRIVATE Util::Generic
    PRIVATE GSPC::share_example_parallel_reduce_module_call_Task
    PRIVATE Boost::base
  INCLUDE_DIRECTORIES PRIVATE "${PROJECT_SOURCE_DIR}/"
)
gspc_add_library (NAME share_example_parallel_reduce_module_call_JobServer
  NAMESPACE GSPC
  TYPE STATIC
  SOURCES "JobServer.cpp"
  LIBRARIES
    PRIVATE Util::RPC
    PRIVATE Util::Generic
    PRIVATE GSPC::share_example_parallel_reduce_module_call_Task
    PRIVATE Boost::base
  INCLUDE_DIRECTORIES PRIVATE "${PROJECT_SOURCE_DIR}/"
)

fhg_add_test (NAME share_example_parallel_reduce_module_call
  SOURCES "driver.cpp"
  USE_BOOST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${CMAKE_CURRENT_SOURCE_DIR}"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
  LIBRARIES gspc
            test-utilities
            Boost::program_options
            Boost::filesystem
            GSPC::share_example_parallel_reduce_module_call_JobServer
  REQUIRES_INSTALLATION
  START_SCOPED_RIF
  COMPILE_DEFINITIONS
    PRIVATE RPC_LIB="$<TARGET_FILE:Util-RPC>"
    PRIVATE JOB_EXECUTOR_LIB="$<TARGET_FILE:GSPC-share_example_parallel_reduce_module_call_JobExecutor>"
    PRIVATE TASK_LIB="$<TARGET_FILE:GSPC-share_example_parallel_reduce_module_call_Task>"
)

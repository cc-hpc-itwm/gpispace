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

gspc_add_library (NAME sdpa
  TYPE STATIC
  SOURCES capability.cpp
          client.cpp
          id_generator.cpp
          requirements_and_preferences.cpp
          com/NetworkStrategy.cpp
          daemon/Agent.cpp
          daemon/GetSchedulerType.cpp
          daemon/Job.cpp
          daemon/Worker.cpp
          daemon/WorkerManager.cpp
          daemon/scheduler/CostAwareWithWorkStealingStrategy.cpp
          daemon/scheduler/CoallocationScheduler.cpp
          daemon/scheduler/GreedyScheduler.cpp
          daemon/scheduler/SingleAllocationScheduler.cpp
          daemon/scheduler/Reservation.cpp
          events/Codec.cpp
          events/EventHandler.cpp
  LIBRARIES PRIVATE GPISpace::header-only
            PUBLIC pnet
            PUBLIC fhgcom
            PUBLIC fhg-util
            PUBLIC gspc::logging
            PUBLIC $<$<BOOL:${GSPC_WITH_IML}>:IML::Client>
            PUBLIC Util::Generic
            PRIVATE scheduler-types
            PUBLIC Boost::thread
            PUBLIC Boost::serialization
            PRIVATE Boost::date_time
)

add_subdirectory (test)
add_subdirectory (test/bin)
add_subdirectory (test/sdpa)

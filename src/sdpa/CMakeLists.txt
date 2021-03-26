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

ADD_LIBRARY(sdpa
  capability.cpp
  client.cpp
  com/NetworkStrategy.cpp
  daemon/Agent.cpp
  daemon/Job.cpp
  daemon/scheduler/CoallocationScheduler.cpp
  daemon/scheduler/Reservation.cpp
  daemon/Worker.cpp
  daemon/WorkerManager.cpp
  events/Codec.cpp
)

target_link_libraries (sdpa
  IML::Client
  RPC
  Boost::serialization
  Boost::date_time
  fhgcom
  Boost::thread
  fhg-util
  pnet
  Util::Generic
  gspc::logging
)

add_subdirectory (test)
add_subdirectory (test/bin)
add_subdirectory (test/sdpa)

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

fhg_add_runtime_executable (NAME gspc-monitor
  SOURCES "monitor.cpp"
          "ui/execution_monitor.cpp"
          "ui/execution_monitor_detail.cpp"
          "ui/execution_monitor_worker_model.cpp"
          "ui/log_monitor.cpp"
  MOC "ui/execution_monitor.hpp"
      "ui/execution_monitor_detail.hpp"
      "ui/execution_monitor_worker_model.hpp"
      "ui/log_monitor.hpp"
  LIBRARIES Boost::thread
            Boost::program_options
            Qt5::Core
            Qt5::Gui
            Qt5::Widgets
            Util::Qt
            fhg-project_configuration
            fhg-util
            fhg_util_qt
            gspc::logging
  CREATE_BUNDLE_INFO
)

add_subdirectory (test)

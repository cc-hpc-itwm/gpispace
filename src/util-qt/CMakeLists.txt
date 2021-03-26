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

include (add_macros)

include (beautify_find_boost)
find_boost (1.59 REQUIRED QUIET COMPONENTS serialization)
find_package (Qt5 COMPONENTS Core Gui Widgets REQUIRED QUIET)

extended_add_library (NAME Qt
  NAMESPACE Util
  SOURCES "do_after_event_loop.cpp"
          "make_drop_target.cpp"
          "style.cpp"
          "$<$<BOOL:${BUILD_TESTING}>:testing/Application.cpp>"
          "$<$<BOOL:${BUILD_TESTING}>:testing/CoreApplication.cpp>"
          "$<$<BOOL:${BUILD_TESTING}>:testing/detail/FakeCommandLineArguments.cpp>"
          "$<$<BOOL:${BUILD_TESTING}>:testing/detail/MinimalPlatformInitializer.cpp>"
          "widget/file_line_edit.cpp"
          "widget/mini_button.cpp"
  MOC "widget/file_line_edit.hpp"
      "widget/mini_button.hpp"
  LIBRARIES Boost::serialization
            Util::Generic
            Qt5::Core
            Qt5::Gui
            Qt5::Widgets
)

function (util_qt_unit_test _path)
  _parse_arguments_with_unknown (ARG "" "" "LIBRARIES;SOURCES" "" ${ARGN})
  string (REPLACE "/" "-" _name "util-qt-${_path}")

  add_unit_test (NAME "${_name}"
    SOURCES "test/${_path}.cpp" ${ARG_SOURCES}
    USE_BOOST
    LIBRARIES Util::Qt ${ARG_LIBRARIES}
    ${ARG_UNPARSED_ARGUMENTS}
  )
endfunction()

util_qt_unit_test (do_after_event_loop)

util_qt_unit_test (qdatastream_enum)

util_qt_unit_test (scoped_signal_block)

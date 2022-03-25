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

extended_add_library (NAME Qt
  NAMESPACE Util
  INCLUDE_DIRECTORIES
    PRIVATE "${PROJECT_SOURCE_DIR}/src"
  SOURCES
    "do_after_event_loop.cpp"
    "make_drop_target.cpp"
    "style.cpp"
    "testing/Application.cpp"
    "testing/CoreApplication.cpp"
    "testing/detail/FakeCommandLineArguments.cpp"
    "testing/detail/MinimalPlatformInitializer.cpp"
    "widget/file_line_edit.cpp"
    "widget/mini_button.cpp"
  MOC
    "${PROJECT_SOURCE_DIR}/include/util-qt/widget/file_line_edit.hpp"
    "${PROJECT_SOURCE_DIR}/include/util-qt/widget/mini_button.hpp"
  LIBRARIES
    PUBLIC
      Util::Qt-Headers
  INSTALL
)
target_compile_features (Util-Qt
  PUBLIC "${_project_cxx_std}"
)

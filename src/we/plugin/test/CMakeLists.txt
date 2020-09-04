# This file is part of GPI-Space.
# Copyright (C) 2020 Fraunhofer ITWM
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

extended_add_library (NAME A
  NAMESPACE PluginTest
  SOURCES "A.cpp"
  POSITION_INDEPENDENT
  TYPE SHARED
  LIBRARIES pnet
)
extended_add_library (NAME B
  NAMESPACE PluginTest
  SOURCES "B.cpp"
  POSITION_INDEPENDENT
  TYPE SHARED
  LIBRARIES pnet
)
extended_add_library (NAME C
  NAMESPACE PluginTest
  SOURCES "C.cpp"
  POSITION_INDEPENDENT
  TYPE SHARED
  LIBRARIES pnet
)
extended_add_library (NAME D
  NAMESPACE PluginTest
  SOURCES "D.cpp"
  POSITION_INDEPENDENT
  TYPE SHARED
  LIBRARIES pnet
)
fhg_add_test (NAME Plugins
  SOURCES test.cpp
  USE_BOOST
  ARGS $<TARGET_FILE:PluginTest::A>
       $<TARGET_FILE:PluginTest::B>
       $<TARGET_FILE:PluginTest::C>
       $<TARGET_FILE:PluginTest::D>
  LIBRARIES gspc
            Util::Generic
  DEPENDS PluginTest::A
          PluginTest::B
          PluginTest::C
          PluginTest::D
)

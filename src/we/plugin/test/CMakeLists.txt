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
  gspc_add_library (NAME A
    NAMESPACE PluginTest
    SOURCES "A.cpp"
    TYPE SHARED
    LIBRARIES GPISpace::header-only
              we-dev
  )
  gspc_add_library (NAME B
    NAMESPACE PluginTest
    SOURCES "B.cpp"
    TYPE SHARED
    LIBRARIES GPISpace::header-only
              we-dev
  )
  gspc_add_library (NAME C
    NAMESPACE PluginTest
    SOURCES "C.cpp"
    TYPE SHARED
    LIBRARIES GPISpace::header-only
              we-dev
  )
  gspc_add_library (NAME D
    NAMESPACE PluginTest
    SOURCES "D.cpp"
    TYPE SHARED
    LIBRARIES GPISpace::header-only
              we-dev
  )
endif()

fhg_add_test (NAME we_Plugins
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

fhg_add_test (NAME "we_plugin_exception_from_plugin_ctor_is_transported_to_users"
  SOURCES "exception_from_plugin_ctor_is_transported_to_users/test.cpp"
  USE_BOOST
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${CMAKE_CURRENT_SOURCE_DIR}"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
       --plugin-path "$<TARGET_FILE:PluginTest::D>"
  LIBRARIES gspc
            test-utilities
            Util::Generic
            Boost::filesystem
            Boost::program_options
  REQUIRES_INSTALLATION
  START_SCOPED_RIF
)

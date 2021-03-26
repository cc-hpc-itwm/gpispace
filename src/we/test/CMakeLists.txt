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
  extended_add_library (NAME test_equality
    NAMESPACE we
    SOURCES "operator_equal.cpp"
    LIBRARIES pnet
  )
endif()

fhg_add_test (NAME we_layer
  SOURCES layer.cpp
  USE_BOOST
  LIBRARIES pnet we::test_equality
)

fhg_add_test (NAME we_layer.ping_pong
  SOURCES layer.ping_pong.cpp
  USE_BOOST
  LIBRARIES pnet Boost::program_options test-utilities
  ARGS --source-directory ${CMAKE_SOURCE_DIR}
)

fhg_add_test (NAME we_layer.performance
  SOURCES layer.performance.cpp
  USE_BOOST
  PERFORMANCE_TEST
  LIBRARIES pnet
)

fhg_add_test (NAME we_require_type
  SOURCES require_type.cpp
  USE_BOOST
  LIBRARIES pnet
)

fhg_add_test (NAME we_signature_of
  SOURCES signature_of.cpp
  USE_BOOST
  LIBRARIES pnet
)

fhg_add_test (NAME we_exception
  SOURCES exception.cpp
  USE_BOOST
  LIBRARIES pnet
)

fhg_add_test (NAME we_context
  SOURCES context.cpp
  USE_BOOST
  LIBRARIES pnet
)

fhg_add_test (NAME we_expression_based_cross_product
  SOURCES expression_based_cross_product.cpp
  USE_BOOST
  LIBRARIES pnet
)

fhg_add_test (NAME we_tp_many_typecheck
  SOURCES tp_many_typecheck.cpp
  USE_BOOST
  LIBRARIES pnet
)

fhg_add_test (NAME we_tp_many_duplicate_connections
  SOURCES tp_many_duplicate_connections.cpp
  USE_BOOST
  LIBRARIES pnet
)

fhg_add_test (NAME we_net
  SOURCES net.cpp
  USE_BOOST
  LIBRARIES pnet
            Util::Generic
)

fhg_add_test (NAME we_net_performance
  SOURCES net.performance.cpp
  USE_BOOST
  LIBRARIES pnet
            Util::Generic
  PERFORMANCE_TEST
)

fhg_add_test (NAME we_eureka_response
  SOURCES eureka_response.cpp
  USE_BOOST
  LIBRARIES pnet
)

add_subdirectory (buffer_alignment)
add_subdirectory (eureka)
add_subdirectory (input_is_copied_to_output_in_module_calls)
add_subdirectory (put_many)
add_subdirectory (transition_cannot_fire_when_input_port_is_not_connected)

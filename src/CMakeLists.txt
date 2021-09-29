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

add_subdirectory (iml)

gspc_add_library (NAME fhgcom
  TYPE STATIC
  SOURCES "fhgcom/address.cpp"
          "fhgcom/channel.cpp"
          "fhgcom/connection.cpp"
          "fhgcom/header.cpp"
          "fhgcom/peer.cpp"
  LIBRARIES
    PRIVATE Util::Generic
    PRIVATE OpenSSL::Crypto
    PRIVATE OpenSSL::SSL
    PUBLIC Boost::filesystem
    PUBLIC Boost::system
    PUBLIC Boost::thread
)
add_subdirectory (fhgcom/tests)

if (GSPC_WITH_MONITOR_APP)
  add_subdirectory (pnete)
  add_subdirectory (util)
endif()

file (READ "we/type/signature/pnetc_compare.hpp" PNETC_COMPARE_HPP)

configure_file ("we/type/signature/cpp.cpp.in"
  "${CMAKE_BINARY_DIR}/we/type/signature/cpp.cpp"
  @ONLY
)
configure_file ("we/type/test/signature.cpp.in"
  "${CMAKE_BINARY_DIR}/we/type/test/signature.cpp"
  @ONLY
)

gspc_add_library (NAME APIGuard
  NAMESPACE GPISpace
  TYPE SHARED
  SOURCES
    "we/loader/api-guard.cpp"
  LIBRARIES
    PUBLIC GPISpace::header-only
    PUBLIC Boost::boost
  INSTALL
    CREATE_BUNDLE_INFO
)

gspc_add_library (NAME pnet
  # Object library for sake of re-exporting in libgspc.
  TYPE OBJECT
  SOURCES "we/exception.cpp"
          "we/expr/eval/context.cpp"
          "we/expr/eval/eval.cpp"
          "we/expr/exception.cpp"
          "we/expr/parse/action.cpp"
          "we/expr/parse/node.cpp"
          "we/expr/parse/parser.cpp"
          "we/expr/token/assoc.cpp"
          "we/expr/token/prec.cpp"
          "we/expr/token/prop.cpp"
          "we/expr/token/tokenizer.cpp"
          "we/expr/token/type.cpp"
          "we/expr/type/AssignResult.cpp"
          "we/expr/type/Context.cpp"
          "we/expr/type/Path.cpp"
          "we/expr/type/Type.cpp"
          "we/expr/type/infer.cpp"
          "we/field.cpp"
          "we/loader/exceptions.cpp"
          "we/loader/loader.cpp"
          "we/loader/module_call.cpp"
          "we/layer.cpp"
          "we/type/Activity.cpp"
          "we/type/MemoryBufferInfo.cpp"
          "we/type/memory_transfer.cpp"
          "we/type/ModuleCall.cpp"
          "we/require_type.cpp"
          "we/signature_of.cpp"
          "we/type/bitsetofint.cpp"
          "we/type/bytearray.cpp"
          "we/type/Expression.cpp"
          "we/type/literal/control.cpp"
          "we/type/net.cpp"
          "we/type/Port.cpp"
          "we/type/property.cpp"
          "we/type/Requirement.cpp"
          "we/type/schedule_data.cpp"
          "we/type/signature/complete.cpp"
          "${CMAKE_BINARY_DIR}/we/type/signature/cpp.cpp"
          "we/type/signature/dump.cpp"
          "we/type/signature/is_literal.cpp"
          "we/type/signature/names.cpp"
          "we/type/signature/resolve.cpp"
          "we/type/signature/show.cpp"
          "we/type/signature/specialize.cpp"
          "we/type/Transition.cpp"
          "we/type/value/function.cpp"
          "we/type/value/name.cpp"
          "we/type/value/path/append.cpp"
          "we/type/value/path/join.cpp"
          "we/type/value/path/split.cpp"
          "we/type/value/peek.cpp"
          "we/type/value/poke.cpp"
          "we/type/value/positions.cpp"
          "we/type/value/read.cpp"
          "we/type/value/show.cpp"
          "we/type/value/serialize.cpp"
          "we/type/value/dump.cpp"

          "we/plugin/Plugin.cpp"
          "we/plugin/Plugins.cpp"

          "we/loader/Module.cpp"
          "we/workflow_response.cpp"

          "xml/parse/error.cpp"
          "xml/parse/parser.cpp"
          "xml/parse/state.cpp"
          "xml/parse/type/connect.cpp"
          "xml/parse/type/expression.cpp"
          "xml/parse/type/function.cpp"
          "xml/parse/type/memory_buffer.cpp"
          "xml/parse/type/memory_transfer.cpp"
          "xml/parse/type/mod.cpp"
          "xml/parse/type/multi_mod.cpp"
          "xml/parse/type/net.cpp"
          "xml/parse/type/place.cpp"
          "xml/parse/type/place_map.cpp"
          "xml/parse/type/port.cpp"
          "xml/parse/type/require.cpp"
          "xml/parse/type/response.cpp"
          "xml/parse/type/eureka.cpp"
          "xml/parse/type/specialize.cpp"
          "xml/parse/type/struct.cpp"
          "xml/parse/type/template.cpp"
          "xml/parse/type/transition.cpp"
          "xml/parse/type/use.cpp"
          "xml/parse/type/with_position_of_definition.cpp"
          "xml/parse/type/preferences.cpp"
          "xml/parse/util/cdata.cpp"
          "xml/parse/util/expect.cpp"
          "xml/parse/util/mk_fstream.cpp"
          "xml/parse/util/name_element.cpp"
          "xml/parse/util/optional.cpp"
          "xml/parse/util/position.cpp"
          "xml/parse/util/property.cpp"
          "xml/parse/util/required.cpp"
          "xml/parse/util/show_node_type.cpp"
          "xml/parse/util/skip.cpp"
          "xml/parse/util/valid_name.cpp"
          "xml/parse/util/validprefix.cpp"
          "xml/parse/util/validstructfield.cpp"
          "xml/parse/util/weparse.cpp"
          "xml/parse/warning.cpp"
  LIBRARIES GPISpace::header-only
            GPISpace::APIGuard
            fhg-util
            drts-context
            IML::Client
            Util::Generic
            Boost::date_time
            Boost::program_options
            Boost::serialization
            RPC
            ${CMAKE_DL_LIBS}
            rapidxml
  COMPILE_DEFINITIONS -DHAS_STD_ALIGN=$<BOOL:${HAS_STD_ALIGN}>
)

add_subdirectory (xml/tests)
add_subdirectory (xml/parse/type/test)

add_subdirectory (we)

add_subdirectory (drts)
add_subdirectory (bin)

add_subdirectory (rpc)

add_subdirectory (rif)
add_subdirectory (sdpa)
add_subdirectory (fhg)
add_subdirectory (util-generic)
if (GSPC_WITH_MONITOR_APP)
  add_subdirectory (util-qt)
endif()

gspc_add_library (NAME logging
  TYPE STATIC
  NAMESPACE gspc
  SOURCES "logging/demultiplexer.cpp"
          "logging/endpoint.cpp"
          "logging/file_sink.cpp"
          "logging/message.cpp"
          "logging/socket_endpoint.cpp"
          "logging/stdout_sink.cpp"
          "logging/stream_emitter.cpp"
          "logging/stream_receiver.cpp"
          "logging/tcp_endpoint.cpp"
          "logging/tcp_server_providing_add_emitters.cpp"
  LIBRARIES rif-started_process_promise
            Util::Generic
            RPC
            Boost::base
            Boost::program_options
)

install (FILES
  logging/endpoint.hpp
  logging/endpoint.ipp
  logging/socket_endpoint.hpp
  logging/socket_endpoint.ipp
  logging/tcp_endpoint.hpp
  logging/tcp_endpoint.ipp
  DESTINATION include/logging
)

gspc_add_executable (NAME gspc-logging-demultiplexer
  SOURCES "logging/gspc-logging-demultiplexer.cpp"
  LIBRARIES gspc::logging
            Util::Generic
            rif-started_process_promise
  CREATE_BUNDLE_INFO
  INSTALL INSTALL_DESTINATION "libexec/gspc"
)

gspc_add_library (NAME logging_test
  TYPE STATIC
  NAMESPACE gspc
  SOURCES "logging/test/message.cpp"
  LIBRARIES gspc::logging
)

gspc_add_executable (NAME gspc-logging-to-stdout
  SOURCES "logging/gspc-logging-to-stdout.cpp"
  LIBRARIES gspc::logging_test
            Util::Generic
  INSTALL INSTALL_DESTINATION "bin"
)

add_unit_test (NAME logging_endpoint
  SOURCES "logging/test/endpoint.cpp"
  LIBRARIES Util::Generic
            gspc::logging_test
  USE_BOOST
)

add_unit_test (NAME logging_file_sink
  SOURCES "logging/test/file_sink.cpp"
  LIBRARIES Util::Generic
            gspc::logging_test
  USE_BOOST
)

add_unit_test (NAME logging_ostream_redirect
  SOURCES "logging/test/ostream_redirect.cpp"
  LIBRARIES Util::Generic
            gspc::logging_test
  USE_BOOST
)

add_unit_test (NAME logging_socket_endpoint
  SOURCES "logging/test/socket_endpoint.cpp"
  LIBRARIES Util::Generic
            gspc::logging_test
  USE_BOOST
)

add_unit_test (NAME logging_stream_emitter
  SOURCES "logging/test/stream_emitter.cpp"
  LIBRARIES Util::Generic
            gspc::logging_test
  USE_BOOST
)

add_unit_test (NAME logging_stream_receiver
  SOURCES "logging/test/stream_receiver.cpp"
  LIBRARIES Util::Generic
            gspc::logging_test
  USE_BOOST
)

add_unit_test (NAME logging_tcp_endpoint
  SOURCES "logging/test/tcp_endpoint.cpp"
  LIBRARIES Util::Generic
            gspc::logging_test
  USE_BOOST
)

install (FILES
  logging/tcp_endpoint.hpp
  DESTINATION include/logging/
)

if (BUILD_TESTING)
  gspc_add_library (NAME we_expr_type_testing
    TYPE STATIC
    SOURCES "we/expr/type/testing/types.cpp"
    LIBRARIES pnet
  )
endif()

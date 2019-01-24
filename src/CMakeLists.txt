# bernd.loerwald@itwm.fraunhofer.de

extended_add_library (NAME fhgcom
  SOURCES "fhgcom/connection.cpp"
          "fhgcom/header.cpp"
          "fhgcom/peer.cpp"
  LIBRARIES fhglog Util::Generic
)
add_subdirectory (fhgcom/tests)

extended_add_library (NAME fhglog
  SOURCES "fhglog/Configuration.cpp"
          "fhglog/Logger.cpp"
          "fhglog/event.cpp"
          "fhglog/level.cpp"
          "fhglog/appender/file.cpp"
          "fhglog/appender/stream.cpp"
          "fhglog/remote/server.cpp"
          "fhglog/remote/appender.cpp"
          LIBRARIES fhg-util Boost::filesystem Util::Generic
)
add_subdirectory (fhglog/appender/test)
add_subdirectory (fhglog/test)

add_subdirectory (gpi-space)
add_subdirectory (vmem)

  add_subdirectory (pnete)
  add_subdirectory (util)

extended_add_library (NAME pnet
  SOURCES "we/exception.cpp"
          "we/expr/eval/context.cpp"
          "we/expr/eval/eval.cpp"
          "we/expr/exception.cpp"
          "we/expr/parse/action.cpp"
          "we/expr/parse/node.cpp"
          "we/expr/parse/parser.cpp"
          "we/expr/parse/simplify/constant_propagation.cpp"
          "we/expr/parse/simplify/copy_propagation.cpp"
          "we/expr/parse/simplify/dead_code_elimination.cpp"
          "we/expr/parse/simplify/expression_list.cpp"
          "we/expr/parse/simplify/simplify.cpp"
          "we/expr/parse/simplify/util.cpp"
          "we/expr/token/assoc.cpp"
          "we/expr/token/prec.cpp"
          "we/expr/token/prop.cpp"
          "we/expr/token/tokenizer.cpp"
          "we/expr/token/type.cpp"
          "we/field.cpp"
          "we/loader/loader.cpp"
          "we/loader/module_call.cpp"
          "we/layer.cpp"
          "we/type/activity.cpp"
          "we/type/module_call.cpp"
          "we/require_type.cpp"
          "we/signature_of.cpp"
          "we/type/bitsetofint.cpp"
          "we/type/bytearray.cpp"
          "we/type/connection.cpp"
          "we/type/copy.cpp"
          "we/type/expression.cpp"
          "we/type/literal/control.cpp"
          "we/type/net.cpp"
          "we/type/port.cpp"
          "we/type/property.cpp"
          "we/type/schedule_data.cpp"
          "we/type/signature/complete.cpp"
          "we/type/signature/cpp.cpp"
          "we/type/signature/dump.cpp"
          "we/type/signature/is_literal.cpp"
          "we/type/signature/names.cpp"
          "we/type/signature/resolve.cpp"
          "we/type/signature/show.cpp"
          "we/type/signature/specialize.cpp"
          "we/type/transition.cpp"
          "we/type/value/function.cpp"
          "we/type/value/name.cpp"
          "we/type/value/name_of.cpp"
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
          "xml/parse/type/net.cpp"
          "xml/parse/type/place.cpp"
          "xml/parse/type/place_map.cpp"
          "xml/parse/type/port.cpp"
          "xml/parse/type/require.cpp"
          "xml/parse/type/response.cpp"
          "xml/parse/type/specialize.cpp"
          "xml/parse/type/struct.cpp"
          "xml/parse/type/template.cpp"
          "xml/parse/type/transition.cpp"
          "xml/parse/type/use.cpp"
          "xml/parse/type/with_position_of_definition.cpp"
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
  LIBRARIES fhg-util
            drts-context
            Util::Generic
            Boost::serialization
            Boost::date_time
            RPC
            ${CMAKE_DL_LIBS}
            rapidxml
)

add_subdirectory (xml/tests)
add_subdirectory (xml/parse/type/test)

add_subdirectory(pnetv)
add_subdirectory (we)
add_subdirectory (mmgr)

add_subdirectory (drts)
add_subdirectory (bin)

add_subdirectory (rpc)

add_subdirectory (rif)
add_subdirectory (sdpa)
add_subdirectory (fhg)
add_subdirectory (util-generic)
add_subdirectory (util-qt)

add_subdirectory (libssh2_wrapper)

add_subdirectory (gspc)

extended_add_library (NAME logging
  NAMESPACE gspc
  SOURCES "logging/file_sink.cpp"
          "logging/legacy/emitter.cpp"
          "logging/legacy/receiver.cpp"
          "logging/legacy_bridge.cpp"
          "logging/message.cpp"
          "logging/stream_emitter.cpp"
          "logging/tcp_endpoint.cpp"
          "logging/tcp_receiver.cpp"
  LIBRARIES fhglog
            Util::Generic
            RPC
            Boost::base
            Boost::program_options
)

add_unit_test (NAME logging_file_sink
  SOURCES "logging/test/file_sink.cpp"
  LIBRARIES Util::Generic
            gspc::logging
  USE_BOOST
)

add_unit_test (NAME logging_legacy_bridge
  SOURCES "logging/test/legacy_bridge.cpp"
  LIBRARIES fhglog
            gspc::logging
            test-utilities
  USE_BOOST
)

add_unit_test (NAME logging_ostream_redirect
  SOURCES "logging/test/ostream_redirect.cpp"
  LIBRARIES Util::Generic
            gspc::logging
  USE_BOOST
)

add_unit_test (NAME logging_stream_emitter
  SOURCES "logging/test/stream_emitter.cpp"
  LIBRARIES Util::Generic
            gspc::logging
  USE_BOOST
)

add_unit_test (NAME logging_tcp_endpoint
  SOURCES "logging/test/tcp_endpoint.cpp"
  LIBRARIES Util::Generic
            gspc::logging
  USE_BOOST
)

add_unit_test (NAME logging_tcp_receiver
  SOURCES "logging/test/tcp_receiver.cpp"
  LIBRARIES Util::Generic
            gspc::logging
  USE_BOOST
)

install (FILES
  logging/tcp_endpoint.hpp
  DESTINATION include/logging/
)

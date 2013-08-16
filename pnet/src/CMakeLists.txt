# mirko.rahn@itwm.fraunhofer.de

include_directories (${CMAKE_CURRENT_SOURCE_DIR})
include_directories (${CMAKE_CURRENT_BINARY_DIR})

include (FindThreads)
if (CMAKE_USE_PTHREADS_INIT)
  link_libraries (${CMAKE_THREAD_LIBS_INIT})
endif()

include (SDPAFindBoost)
include_directories (SYSTEM ${Boost_INCLUDE_DIRS})

find_package (FhgLog REQUIRED QUIET)
include_directories (${FhgLog_INCLUDE_DIR})

find_package (UTIL REQUIRED QUIET)
include_directories (${UTIL_INCLUDE_DIR})

add_library (we-dev SHARED
  we/exception.cpp
  we/expr/token/type.cpp
  we/field.cpp
  we/signature_of.cpp
  we/type/bitsetofint.cpp
  we/type/bytearray.cpp
  we/type/literal/control.cpp
  we/type/signature/show.cpp
  we/type/value/name.cpp
  we/type/value/name_of.cpp
  we/type/value/path/append.cpp
  we/type/value/path/join.cpp
  we/type/value/path/split.cpp
  we/type/value/peek.cpp
  we/type/value/poke.cpp
  we/type/value/show.cpp
)

set_target_properties(we-dev
  PROPERTIES VERSION 1
)
target_link_libraries(we-dev fhg-util)

install (TARGETS we-dev
  LIBRARY DESTINATION lib
  COMPONENT runtime
)

add_library (pnet
  we/container/priostore.cpp
  we/exception.cpp
  we/expr/eval/context.cpp
  we/expr/eval/eval.cpp
  we/expr/eval/refnode.cpp
  we/expr/exception.cpp
  we/expr/parse/action.cpp
  we/expr/parse/node.cpp
  we/expr/parse/parser.cpp
  we/expr/parse/simplify/constant_propagation.cpp
  we/expr/parse/simplify/copy_propagation.cpp
  we/expr/parse/simplify/dead_code_elimination.cpp
  we/expr/parse/simplify/expression_list.cpp
  we/expr/parse/simplify/simplify.cpp
  we/expr/parse/simplify/util.cpp
  we/expr/parse/util/get_names.cpp
  we/expr/token/assoc.cpp
  we/expr/token/prec.cpp
  we/expr/token/prop.cpp
  we/expr/token/tokenizer.cpp
  we/expr/token/type.cpp
  we/field.cpp
  we/loader/loader.cpp
  we/mgmt/bits/descriptor.cpp
  we/mgmt/type/activity.cpp
  we/mgmt/type/flags.cpp
  we/require_type.cpp
  we/signature_of.cpp
  we/type/bitsetofint.cpp
  we/type/bytearray.cpp
  we/type/connection.cpp
  we/type/expression.cpp
  we/type/id.cpp
  we/type/literal/control.cpp
  we/type/net.cpp
  we/type/place.cpp
  we/type/port.cpp
  we/type/property.cpp
  we/type/schedule_data.cpp
  we/type/signature/complete.cpp
  we/type/signature/cpp.cpp
  we/type/signature/dump.cpp
  we/type/signature/is_literal.cpp
  we/type/signature/name.cpp
  we/type/signature/names.cpp
  we/type/signature/resolve.cpp
  we/type/signature/show.cpp
  we/type/signature/signature.cpp
  we/type/signature/specialize.cpp
  we/type/transition.cpp
  we/type/value/function.cpp
  we/type/value/name.cpp
  we/type/value/name_of.cpp
  we/type/value/of_type.cpp
  we/type/value/path/append.cpp
  we/type/value/path/join.cpp
  we/type/value/path/split.cpp
  we/type/value/peek.cpp
  we/type/value/poke.cpp
  we/type/value/read.cpp
  we/type/value/show.cpp
  we/util/cross.cpp

  xml/parse/error.cpp
  xml/parse/id/generic.cpp
  xml/parse/id/mapper.cpp
  xml/parse/id/types.cpp
  xml/parse/parser.cpp
  xml/parse/state.cpp
  xml/parse/type/connect.cpp
  xml/parse/type/expression.cpp
  xml/parse/type/function.cpp
  xml/parse/type/mod.cpp
  xml/parse/type/net.cpp
  xml/parse/type/place.cpp
  xml/parse/type/place_map.cpp
  xml/parse/type/port.cpp
  xml/parse/type/require.cpp
  xml/parse/type/specialize.cpp
  xml/parse/type/struct.cpp
  xml/parse/type/template.cpp
  xml/parse/type/transition.cpp
  xml/parse/type/use.cpp
  xml/parse/type/link.cpp
  xml/parse/type/with_position_of_definition.cpp
  xml/parse/util/mk_fstream.cpp
  xml/parse/util/valid_name.cpp
  xml/parse/util/cdata.cpp
  xml/parse/util/expect.cpp
  xml/parse/util/name_element.cpp
  xml/parse/util/optional.cpp
  xml/parse/util/required.cpp
  xml/parse/util/skip.cpp
  xml/parse/util/validprefix.cpp
  xml/parse/util/validstructfield.cpp
  xml/parse/util/property.cpp
  xml/parse/util/show_node_type.cpp
  xml/parse/util/weparse.cpp
  xml/parse/util/position.cpp
  xml/parse/warning.cpp
)

target_link_libraries (pnet fhg-util)

install (FILES
  we/expr/eval/context.hpp
  DESTINATION include/we/expr/eval
  COMPONENT sdk
)
install (FILES
  we/expr/token/type.hpp
  DESTINATION include/we/expr/token
  COMPONENT sdk
)
install (FILES
  we/loader/IModule.hpp
  we/loader/api-guard.hpp
  we/loader/macros.hpp
  we/loader/types.hpp
  DESTINATION include/we/loader
  COMPONENT sdk
)
install (FILES
  we/type/bitsetofint.hpp
  we/type/bytearray.hpp
  we/type/value.hpp
  we/type/signature.hpp
  DESTINATION include/we/type
  COMPONENT sdk
)
install (FILES
  we/type/value/poke.hpp
  we/type/value/show.hpp
  DESTINATION include/we/type/value
  COMPONENT sdk
)
install (FILES
  we/field.hpp
  we/signature_of.hpp
  we/exception.hpp
  DESTINATION include/we/
  COMPONENT sdk
)
install (FILES
  we/type/literal/control.hpp
  DESTINATION include/we/type/literal
  COMPONENT sdk
)

set (wfhd_srcs
  we/util/wfhd.c
)
set (wfhd_hdrs
  we/util/wfhd.h
)

add_library (wfhd SHARED ${wfhd_srcs})
set_target_properties (wfhd PROPERTIES VERSION 1)
install (TARGETS wfhd
  LIBRARY DESTINATION lib
  COMPONENT runtime
  )
install (FILES ${wfhd_hdrs}
  DESTINATION include/we/util
  COMPONENT runtime-headers
)

add_executable (generate_parser_action_table we/expr/parse/table.cpp)
target_link_libraries (generate_parser_action_table pnet)

if (BUILD_TESTING)
  add_executable (test_we_type_bytearray we/type/test/test_bytearray.cpp)
  target_link_libraries(test_we_type_bytearray pnet)
  target_link_libraries(test_we_type_bytearray ${Boost_UNIT_TEST_LIBRARIES})
  target_link_libraries(test_we_type_bytearray ${Boost_LIBRARIES})
  add_test ("test_we_type_bytearray" test_we_type_bytearray)
endif (BUILD_TESTING)

add_executable(pnetc xml/parse/pnetc.cpp)
target_link_libraries(pnetc fhg-revision)
target_link_libraries(pnetc pnet)
target_link_libraries(pnetc ${Boost_LIBRARIES})

install (TARGETS pnetc RUNTIME DESTINATION bin COMPONENT runtime)

if (ENABLE_STATIC_BINARIES)
  add_executable(pnetc.static xml/parse/pnetc.cpp)
  target_link_libraries(pnetc.static fhg-revision)
  target_link_libraries(pnetc.static pnet)
  target_link_libraries(pnetc.static ${Boost_LIBRARIES})

  set_target_properties(pnetc.static PROPERTIES LINK_FLAGS "-static")

  install (TARGETS pnetc.static RUNTIME DESTINATION bin COMPONENT runtime)
endif (ENABLE_STATIC_BINARIES)

fhg_add_test (we/type/test/value.cpp
  PROJECT test_we_type
  LINK_LIBRARIES pnet
)

fhg_add_test (we/type/test/signature.cpp
  PROJECT test_we_type
  LINK_LIBRARIES pnet
)

fhg_add_test (we/test/require_type.cpp
  PROJECT test_we
  LINK_LIBRARIES pnet
)

fhg_add_test (we/test/signature_of.cpp
  PROJECT test_we
  LINK_LIBRARIES pnet
)

fhg_add_test (we/test/exception.cpp
  PROJECT test_we
  LINK_LIBRARIES pnet
)

add_executable (mk_cpp we/type/test/mk_cpp.cpp)
target_link_libraries (mk_cpp pnet ${Boost_LIBRARIES})

add_custom_command (
  OUTPUT sig.hpp sig.cpp
  COMMAND ${CMAKE_CURRENT_BINARY_DIR}/mk_cpp -H sig.hpp -I sig.cpp
  DEPENDS mk_cpp
)

include_directories (${CMAKE_CURRENT_BINARY_DIR})
add_library (sig ${CMAKE_CURRENT_BINARY_DIR}/sig.cpp)

fhg_add_test (we/type/test/sig.cpp
  PROJECT test_we
  LINK_LIBRARIES sig pnet
)

if (BUILD_TESTING)
  add_subdirectory (tests)
  add_subdirectory (xml/tests)
  add_subdirectory (we/container/test)
  add_subdirectory (we/util/test)
endif()

add_subdirectory (apps)

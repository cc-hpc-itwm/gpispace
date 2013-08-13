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

add_subdirectory (apps)

add_library (we2
  we2/exception.cpp
  we2/field.cpp
  we2/require_type.cpp
  we2/signature_of.cpp
  we2/type/signature/complete.cpp
  we2/type/signature/cpp.cpp
  we2/type/signature/dump.cpp
  we2/type/signature/is_literal.cpp
  we2/type/signature/name.cpp
  we2/type/signature/show.cpp
  we2/type/signature/signature.cpp
  we2/type/value/name.cpp
  we2/type/value/name_of.cpp
  we2/type/value/of_type.cpp
  we2/type/value/path/append.cpp
  we2/type/value/path/join.cpp
  we2/type/value/path/split.cpp
  we2/type/value/peek.cpp
  we2/type/value/poke.cpp
  we2/type/value/read.cpp
  we2/type/value/show.cpp

  we2/type/compat.cpp
  we2/type/compat.sig.cpp
)
target_link_libraries (we2 fhg-util)

fhg_add_test (we2/type/test/value.cpp
  PROJECT test_we_type
  LINK_LIBRARIES we2 we-dev
)

fhg_add_test (we2/type/test/signature.cpp
  PROJECT test_we_type
  LINK_LIBRARIES we2 we-dev
)

fhg_add_test (we2/type/test/compat.cpp
  PROJECT test_we_compat
  LINK_LIBRARIES we2 we-dev
)

fhg_add_test (we2/test/require_type.cpp
  PROJECT test_we
  LINK_LIBRARIES we2 we-dev
)

fhg_add_test (we2/test/signature_of.cpp
  PROJECT test_we
  LINK_LIBRARIES we2 we-dev
)

fhg_add_test (we2/test/exception.cpp
  PROJECT test_we
  LINK_LIBRARIES we2 we-dev
)

add_executable (mk_cpp we2/type/test/mk_cpp.cpp)
target_link_libraries (mk_cpp we2 we-dev ${Boost_LIBRARIES})

add_custom_command (
  OUTPUT sig.hpp sig.cpp
  COMMAND ${CMAKE_CURRENT_BINARY_DIR}/mk_cpp -H sig.hpp -I sig.cpp
  DEPENDS mk_cpp
)

include_directories (${CMAKE_CURRENT_BINARY_DIR})
add_library (sig ${CMAKE_CURRENT_BINARY_DIR}/sig.cpp)

fhg_add_test (we2/type/test/sig.cpp
  PROJECT test_we
  LINK_LIBRARIES sig we2 we-dev
)

set (we_dev_srcs
  we/type/bitsetofint.cpp
  we/type/bytearray.cpp
  we/type/literal/control.cpp
  we2/exception.cpp
  we2/field.cpp
  we2/signature_of.cpp
  we2/type/compat.cpp
  we2/type/value/poke.cpp
)

set (we_srcs
  we/container/priostore.cpp
  we/expr/eval/context.cpp
  we/expr/eval/eval.cpp
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
  we/loader/loader.cpp
  we/mgmt/bits/descriptor.cpp
  we/mgmt/type/activity.cpp
  we/mgmt/type/flags.cpp
  we/type/connection.cpp
  we/type/expression.cpp
  we/type/id.cpp
  we/type/net.cpp
  we/type/place.cpp
  we/type/port.cpp
  we/type/property.cpp
  we/type/schedule_data.cpp
  we/util/cross.cpp

  ${we_dev_srcs}

  we/type/literal/read.cpp
  we/type/literal/valid_name.cpp
  we/type/literal/default.cpp
  we/type/literal/name.cpp
  we/type/literal/function.cpp
  we/type/literal/show.cpp
  we/type/literal/require_type.cpp
  we/type/literal/cpp.cpp
  we/type/value/read.cpp
  we/type/value/show.cpp
  we/type/value/function.cpp
  we/expr/eval/refnode.cpp
  we/type/transition.cpp
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

add_subdirectory (we/container/test)
add_subdirectory (we/util/test)

if (BUILD_TESTING)
  add_executable (test_we_type_bytearray we/type/test/test_bytearray.cpp)
  target_link_libraries(test_we_type_bytearray pnet)
  target_link_libraries(test_we_type_bytearray ${Boost_UNIT_TEST_LIBRARIES})
  target_link_libraries(test_we_type_bytearray ${Boost_LIBRARIES})
  add_test ("test_we_type_bytearray" test_we_type_bytearray)
endif (BUILD_TESTING)

install (FILES
  we/expr/eval/context.hpp
  DESTINATION include/we/expr/eval
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
  we/type/literal.hpp
  we/type/value.hpp
  DESTINATION include/we/type
  COMPONENT sdk
)
install (FILES
  we2/type/value.hpp
  we2/type/signature.hpp
  DESTINATION include/we2/type
  COMPONENT sdk
)
install (FILES
  we2/type/value/poke.hpp
  we2/type/value/show.hpp
  DESTINATION include/we2/type/value
  COMPONENT sdk
)
install (FILES
  we2/field.hpp
  we2/signature_of.hpp
  we2/exception.hpp
  DESTINATION include/we2/
  COMPONENT sdk
)

# BEGIN compat
install (FILES
  we2/type/compat.hpp
  DESTINATION include/we2/type
  COMPONENT sdk
)
# END compat

install (FILES
  we/type/literal/control.hpp
  we/type/literal/show.hpp
  DESTINATION include/we/type/literal
  COMPONENT sdk
)
install (FILES
  DESTINATION include/we/type/signature
  COMPONENT sdk
)
install (FILES
  we/type/value/cpp/get.hpp
  DESTINATION include/we/type/value/cpp
  COMPONENT sdk
)
install (FILES
  we/type/value/missing_binding.hpp
  DESTINATION include/we/type/value
  COMPONENT sdk
)

add_executable(pnetc xml/parse/pnetc.cpp)
target_link_libraries(pnetc fhg-revision)
target_link_libraries(pnetc pnet)
target_link_libraries(pnetc we2)
target_link_libraries(pnetc ${Boost_LIBRARIES})

install (TARGETS pnetc RUNTIME DESTINATION bin COMPONENT runtime)

if (ENABLE_STATIC_BINARIES)
  add_executable(pnetc.static xml/parse/pnetc.cpp)
  target_link_libraries(pnetc.static fhg-revision)
  target_link_libraries(pnetc.static pnet)
  target_link_libraries(pnetc.static we2)
  target_link_libraries(pnetc.static ${Boost_LIBRARIES})

  set_target_properties(pnetc.static PROPERTIES LINK_FLAGS "-static")

  install (TARGETS pnetc.static RUNTIME DESTINATION bin COMPONENT runtime)
endif (ENABLE_STATIC_BINARIES)

set (xml_sources
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

add_library (pnet ${we_srcs} ${xml_sources})
target_link_libraries (pnet we2)

add_library (we-dev SHARED ${we_dev_srcs})
set_target_properties(we-dev
  PROPERTIES VERSION 1
)
target_link_libraries(we-dev fhg-util we2)

install (TARGETS we-dev
  LIBRARY DESTINATION lib
  COMPONENT runtime
)

if (BUILD_TESTING)
  add_subdirectory (tests)
  add_subdirectory (xml/tests)
endif()

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

set (we_dev_srcs
  we/type/bitsetofint.cpp
  we/type/bytearray.cpp
  we/type/literal/control.cpp
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
  we/type/value/field.cpp
  we/type/value/read.cpp
  we/type/value/show.cpp
  we/type/value/require_type.cpp
  we/type/value/mk_structured.cpp
  we/type/value/put.cpp
  we/type/value/function.cpp
  we/type/value/get.cpp
  we/type/value/find.cpp
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
  we/type/literal/control.hpp
  we/type/literal/show.hpp
  DESTINATION include/we/type/literal
  COMPONENT sdk
)
install (FILES
  we/type/signature/types.hpp
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

add_subdirectory (xml/parse)

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
target_link_libraries (pnet
  fhg-util
)
add_library (pnet-shared SHARED ${we_srcs} ${xml_sources})
set_target_properties(pnet-shared PROPERTIES OUTPUT_NAME pnet)

add_library (we-dev SHARED ${we_dev_srcs})
set_target_properties(we-dev
  PROPERTIES VERSION 1
)

install (TARGETS we-dev
  LIBRARY DESTINATION lib
  COMPONENT runtime
)

if (BUILD_TESTING)
  add_subdirectory (tests)
  add_subdirectory (xml/tests)
endif()

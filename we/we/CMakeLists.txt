
set (we_dev_srcs
  type/bitsetofint.cpp
  type/bytearray.cpp
  type/literal/control.cpp
)

set (we_srcs
  expr/eval/context.cpp
  expr/eval/eval.cpp
  expr/exception.cpp
  expr/parse/action.cpp
  expr/parse/node.cpp
  expr/parse/parser.cpp
  expr/parse/simplify/constant_propagation.cpp
  expr/parse/simplify/copy_propagation.cpp
  expr/parse/simplify/dead_code_elimination.cpp
  expr/parse/simplify/expression_list.cpp
  expr/parse/simplify/simplify.cpp
  expr/parse/simplify/util.cpp
  expr/parse/util/get_names.cpp
  expr/token/assoc.cpp
  expr/token/prec.cpp
  expr/token/prop.cpp
  expr/token/tokenizer.cpp
  expr/token/type.cpp
  loader/loader.cpp
  mgmt/bits/descriptor.cpp
  mgmt/type/activity.cpp
  mgmt/type/flags.cpp
  type/connection.cpp
  type/expression.cpp
  type/id.cpp
  type/net.cpp
  type/place.cpp
  type/port.cpp
  type/property.cpp
  util/cross.cpp

  ${we_dev_srcs}

  type/literal/read.cpp
  type/literal/valid_name.cpp
  type/literal/default.cpp
  type/literal/name.cpp
  type/literal/function.cpp
  type/literal/show.cpp
  type/literal/require_type.cpp
  type/literal/cpp.cpp
  type/value/field.cpp
  type/value/read.cpp
  type/value/show.cpp
  type/value/require_type.cpp
  type/value/mk_structured.cpp
  type/value/put.cpp
  type/value/function.cpp
  type/value/get.cpp
  type/value/find.cpp
  expr/eval/refnode.cpp
  type/transition.cpp
)

add_library (we ${we_srcs})
add_library (we-shared SHARED ${we_srcs})
set_target_properties(we-shared PROPERTIES OUTPUT_NAME we)

add_library (we-dev ${we_dev_srcs})
add_library (we-dev-shared SHARED ${we_dev_srcs})
set_target_properties(we-dev-shared PROPERTIES OUTPUT_NAME we-dev)

target_link_libraries (we
  fhg-util
)

install (TARGETS we-dev
  ARCHIVE DESTINATION lib
  COMPONENT runtime
)

install (TARGETS we-dev-shared
  LIBRARY DESTINATION lib
  COMPONENT runtime
)

add_executable (generate_parser_action_table expr/parse/table.cpp)
target_link_libraries (generate_parser_action_table we)

if (BUILD_TESTING)
  add_executable (test_we_type_bytearray type/test/test_bytearray.cpp)
  target_link_libraries(test_we_type_bytearray we)
  target_link_libraries(test_we_type_bytearray ${Boost_UNIT_TEST_LIBRARIES})
  target_link_libraries(test_we_type_bytearray ${Boost_LIBRARIES})
  add_test ("test_we_type_bytearray" test_we_type_bytearray)
endif (BUILD_TESTING)

install (FILES
  util/token.hpp
  DESTINATION include/we/util
  COMPONENT headers
)

install (FILES
  container/adjacency.hpp
  container/priostore.hpp
  container/svector.hpp
  DESTINATION include/we/container
  COMPONENT headers
)

install (FILES
  loader/api-guard.hpp
  loader/exceptions.hpp
  loader/IModule.hpp
  loader/macros.hpp
  loader/module_call.hpp
  loader/Module.hpp
  loader/module_traits.hpp
  loader/putget.hpp
  loader/types.hpp
  DESTINATION include/we/loader
  COMPONENT headers
)

install (FILES
  serialize/unordered_map.hpp
  serialize/unordered_set.hpp
  DESTINATION include/we/serialize
  COMPONENT headers
)

install (FILES
  mgmt/basic_layer.hpp
  mgmt/context.hpp
  mgmt/exception.hpp
  mgmt/layer.hpp
  DESTINATION include/we/mgmt
  COMPONENT headers
)

install (FILES
  mgmt/bits/commands.hpp
  mgmt/bits/execution_policy.hpp
  mgmt/bits/queue.hpp
  mgmt/bits/set.hpp
  mgmt/bits/signal.hpp
  DESTINATION include/we/mgmt/bits/
  COMPONENT headers
)

install (FILES
  type/bits/transition/optimize.hpp
  type/bits/transition/toDot.hpp
  DESTINATION include/we/type/bits/transition
  COMPONENT headers
)

install (FILES
  type/bits/transition/optimize/is_associated.hpp
  type/bits/transition/optimize/merge_expressions.hpp
  type/bits/transition/optimize/merge_places.hpp
  type/bits/transition/optimize/option.hpp
  type/bits/transition/optimize/simple_pipe_elimination.hpp
  DESTINATION include/we/type/bits/transition/optimize
  COMPONENT headers
)

install (FILES
  type/condition.hpp
  type/error.hpp
  type/literal.hpp
  type/module_call.hpp
  type/requirement.hpp
  type/signature.hpp
  type/transition.hpp
  type/value.hpp
  DESTINATION include/we/type
  COMPONENT headers
)

install (FILES
  type/signature/cpp.hpp
  type/signature/types.hpp
  DESTINATION include/we/type/signature
  COMPONENT headers
)


add_library (we
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
  type/literal/control.cpp
  type/expression.cpp
  type/id.cpp
  type/net.cpp
  type/place.cpp
  type/port.cpp
  type/property.cpp
  util/cross.cpp
  type/bitsetofint.cpp
  type/bytearray.cpp
  type/literal/read.cpp
)

target_link_libraries (we
  fhg-util
  ${Boost_LIBRARIES}
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
  type/token.hpp
  type/transition.hpp
  type/value.hpp
  DESTINATION include/we/type
  COMPONENT headers
)

install (FILES
  type/literal/cpp.hpp
  type/literal/default.hpp
  type/literal/function.hpp
  type/literal/name.hpp
  type/literal/require_type.hpp
  type/literal/show.hpp
  type/literal/valid_name.hpp
  DESTINATION include/we/type/literal
  COMPONENT headers
)

install (FILES
  type/signature/cpp.hpp
  type/signature/types.hpp
  DESTINATION include/we/type/signature
  COMPONENT headers
)

install (FILES
  type/value/container/bind.hpp
  type/value/container/container.hpp
  type/value/container/exception.hpp
  type/value/container/show.hpp
  type/value/container/type.hpp
  type/value/container/value.hpp
  DESTINATION include/we/type/value/container
  COMPONENT headers
)

install (FILES
  type/value/cpp/get.hpp
  DESTINATION include/we/type/value/cpp
  COMPONENT headers
)

install (FILES
  type/value/eq.hpp
  type/value/field.hpp
  type/value/function.hpp
  type/value/get.hpp
  type/value/hash.hpp
  type/value/mk_structured.hpp
  type/value/put.hpp
  type/value/read.hpp
  type/value/require_type.hpp
  type/value/show.hpp
  DESTINATION include/we/type/value
  COMPONENT headers
)

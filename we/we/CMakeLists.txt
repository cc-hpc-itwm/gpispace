# -*- mode: cmake; -*-

add_subdirectory (container)
add_subdirectory (expr)
add_subdirectory (mgmt)
add_subdirectory (serialize)
add_subdirectory (type)
add_subdirectory (util)
add_subdirectory (loader)

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
  type/expression.cpp
  type/id.cpp
  type/net.cpp
  type/place.cpp
  type/port.cpp
  type/property.cpp
  util/cross.cpp
)

target_link_libraries (we
  fhg-util
  ${Boost_LIBRARIES}
)

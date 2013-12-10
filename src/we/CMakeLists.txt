# {mirko.rahn,bernd.loerwald}@itwm.fraunhofer.de

add_library (we-dev SHARED
  exception.cpp
  expr/token/type.cpp
  field.cpp
  signature_of.cpp
  type/bitsetofint.cpp
  type/bytearray.cpp
  type/literal/control.cpp
  type/signature/show.cpp
  type/value/name.cpp
  type/value/name_of.cpp
  type/value/path/append.cpp
  type/value/path/join.cpp
  type/value/path/split.cpp
  type/value/peek.cpp
  type/value/poke.cpp
  type/value/show.cpp
  type/value/read.cpp

  loader/Module.cpp
)

target_link_libraries(we-dev fhg-util ${CMAKE_DL_LIBS})
set_target_properties (we-dev PROPERTIES OUTPUT_NAME we-dev VERSION 1)

install (TARGETS we-dev
  LIBRARY DESTINATION lib
  COMPONENT runtime
)

install (FILES
  expr/eval/context.hpp
  DESTINATION include/we/expr/eval
  COMPONENT sdk
)
install (FILES
  expr/token/type.hpp
  DESTINATION include/we/expr/token
  COMPONENT sdk
)
install (FILES
  loader/IModule.hpp
  loader/api-guard.hpp
  loader/macros.hpp
  DESTINATION include/we/loader
  COMPONENT sdk
)
install (FILES
  type/bitsetofint.hpp
  type/bytearray.hpp
  type/value.hpp
  type/signature.hpp
  DESTINATION include/we/type
  COMPONENT sdk
)
install (FILES
  type/value/poke.hpp
  type/value/read.hpp
  type/value/show.hpp
  type/value/to_value.hpp
  type/value/wrap.hpp
  type/value/unwrap.hpp
  DESTINATION include/we/type/value
  COMPONENT sdk
)
install (FILES
  type/value/path/append.hpp
  DESTINATION include/we/type/value/path
  COMPONENT sdk
)
install (FILES
  field.hpp
  signature_of.hpp
  exception.hpp
  DESTINATION include/we/
  COMPONENT sdk
)
install (FILES
  type/literal/control.hpp
  DESTINATION include/we/type/literal
  COMPONENT sdk
)

add_library (wfhd SHARED util/wfhd.c)
set_target_properties (wfhd PROPERTIES OUTPUT_NAME wfhd VERSION 1)

install (TARGETS wfhd
  LIBRARY DESTINATION lib
  COMPONENT runtime
  )
install (FILES
  util/wfhd.h
  DESTINATION include/we/util
  COMPONENT runtime-headers
)

add_executable (generate_parser_action_table expr/parse/table.cpp)
target_link_libraries (generate_parser_action_table pnet)

fhg_add_test (expr/parse/test/get_names.cpp
  PROJECT test_we_expr_parse
  LINK_LIBRARIES pnet
)

fhg_add_test (expr/parse/test/simplify.cpp
  BOOST_UNIT_TEST
  PROJECT test_we_expr_parse
  LINK_LIBRARIES pnet
)

fhg_add_test (expr/test/rename.cpp
  PROJECT test_we_expr
  LINK_LIBRARIES pnet
)

add_subdirectory (mgmt/bits/test)

add_subdirectory (loader/test)

add_subdirectory (container/test)
add_subdirectory (util/test)
add_subdirectory (test)
add_subdirectory (type/test)

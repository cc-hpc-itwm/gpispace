# -*- mode: cmake; -*-
set (hdrs
  join.hpp
  maybe.hpp
  read_bool.hpp
  read.hpp
  show.hpp
  split.hpp
  starts_with.hpp
  util.hpp
  warnings.hpp

  filesystem.hpp
  cpp.hpp

  ini-parser.hpp
  ini-parser.tcc
  ini-parser-except.hpp
  ini-parser-helper.hpp

  getenv.hpp

  xml.hpp
  bool.hpp
  bool_io.hpp

  random.hpp
  url.hpp
)

install(FILES ${hdrs}
  DESTINATION include/fhg/util
  COMPONENT headers)

add_subdirectory(parse)
add_subdirectory(thread)
add_subdirectory(filesystem)
add_subdirectory(cpp)
add_subdirectory(test)

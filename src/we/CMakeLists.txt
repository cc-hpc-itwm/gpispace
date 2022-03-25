# This file is part of GPI-Space.
# Copyright (C) 2022 Fraunhofer ITWM
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

gspc_add_library (NAME we-dev
  TYPE SHARED
  SOURCES "exception.cpp"
          "expr/token/type.cpp"
          "field.cpp"
          "signature_of.cpp"
          "type/bitsetofint.cpp"
          "type/bytearray.cpp"
          "type/literal/control.cpp"
          "type/signature/show.cpp"
          "type/value/name.cpp"
          "type/value/path/append.cpp"
          "type/value/path/join.cpp"
          "type/value/path/split.cpp"
          "type/value/peek.cpp"
          "type/value/poke.cpp"
          "type/value/show.cpp"
          "type/value/read.cpp"
          "type/value/serialize.cpp"

          "loader/Module.cpp"
          "loader/exceptions.cpp"
  LIBRARIES GPISpace::header-only
            fhg-util
            ${CMAKE_DL_LIBS}
  INSTALL
  CREATE_BUNDLE_INFO
)

if (BUILD_TESTING)
  gspc_add_library (NAME we-dev-testing
    TYPE STATIC
    SOURCES "expr/token/testing/all_tokens.cpp"
    LIBRARIES we-dev
  )
endif()

install (FILES
  expr/eval/context.hpp
  DESTINATION include/we/expr/eval
)
install (FILES
  expr/token/type.hpp
  DESTINATION include/we/expr/token
)
install (FILES
  loader/IModule.hpp
  loader/api-guard.hpp
  loader/macros.hpp
  DESTINATION include/we/loader
)
install (FILES
  type/bitsetofint.hpp
  type/bytearray.hpp
  type/value.hpp
  type/signature.hpp
  DESTINATION include/we/type
)
install (FILES
  type/value/peek.hpp
  type/value/peek_or_die.hpp
  type/value/poke.hpp
  type/value/read.hpp
  type/value/serialize.hpp
  type/value/show.hpp
  type/value/to_value.hpp
  type/value/from_value.hpp
  type/value/wrap.hpp
  type/value/unwrap.hpp
  DESTINATION include/we/type/value
)
install (FILES
  type/signature/show.hpp
  DESTINATION include/we/type/signature
)
install (FILES
  type/value/path/append.hpp
  type/value/path/join.hpp
  DESTINATION include/we/type/value/path
)
install (FILES
  field.hpp
  signature_of.hpp
  exception.hpp
  DESTINATION include/we/
)
install (FILES
  type/literal/control.hpp
  DESTINATION include/we/type/literal
)
install (FILES
  plugin/Base.hpp
  DESTINATION include/we/plugin
)

add_subdirectory (expr/parse/test)
add_subdirectory (expr/test)
add_subdirectory (loader/test)
add_subdirectory (plugin/test)
add_subdirectory (test)
add_subdirectory (type/test)

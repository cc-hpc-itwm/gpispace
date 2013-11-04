include (cmake_parse_arguments)
include (car_cdr_macros)

find_package (source_highlite QUIET)
find_package (asciidoc QUIET)

macro (ADD_DOC)
  parse_arguments (DOC
    "INPUT;OUTPUT;BACKEND"
    ""
    ${ARGN}
  )
  car (DOC_TARGET ${DOC_DEFAULT_ARGS})

  if (ASCIIDOC_FOUND AND SOURCE_HIGHLITE_FOUND)
    add_custom_command (
      OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${DOC_OUTPUT}
      COMMAND "PATH=$ENV{PATH}:${SOURCE_HIGHLITE_BINDIR}"
              ${ASCIIDOC_EXECUTABLE}
              -b ${DOC_BACKEND}
              -o ${CMAKE_CURRENT_BINARY_DIR}/${DOC_OUTPUT}
              ${CMAKE_CURRENT_SOURCE_DIR}/${DOC_INPUT}
      )
    add_custom_target (${DOC_TARGET} DEPENDS ${DOC_OUTPUT})
  endif()
endmacro()

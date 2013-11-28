include (cmake_parse_arguments)
include (car_cdr_macros)

find_package (source_highlite QUIET)
find_package (asciidoc QUIET)

macro (ADD_ASCIIDOC)
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
      DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${DOC_INPUT}
    )
    add_custom_target (${DOC_TARGET}
      ALL
      DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${DOC_OUTPUT}
    )
  endif()
endmacro()

macro (ADD_A2X)
  parse_arguments (DOC
    "INPUT;FORMAT;OPTION"
    ""
    ${ARGN}
  )
  car (DOC_TARGET ${DOC_DEFAULT_ARGS})

  if (A2X_FOUND AND SOURCE_HIGHLITE_FOUND)
    add_custom_command (
      OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/a2x-html-${DOC_FORMAT}
      COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/a2x-html &&
              ${A2X_EXECUTABLE}
             -f ${DOC_FORMAT} ${DOC_OPTION}
             --no-xmllint
             --destination-dir=${CMAKE_CURRENT_BINARY_DIR}/a2x-html
             ${CMAKE_CURRENT_SOURCE_DIR}/${DOC_INPUT}
      DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${DOC_INPUT}
    )
    add_custom_target (${DOC_TARGET}
      ALL
      DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/a2x-html-${DOC_FORMAT}
    )
  endif()
endmacro()

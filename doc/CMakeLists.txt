if (ENABLE_DOCUMENTATION)
  include (cmake_parse_arguments)
  include (car_cdr_macros)

  find_package (source_highlite REQUIRED QUIET)
  find_package (asciidoc 8.6.7 REQUIRED QUIET)

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
                ${CMAKE_CURRENT_SOURCE_DIR}/${DOC_INPUT} ||
                rm -rf ${CMAKE_CURRENT_BINARY_DIR}/${DOC_OUTPUT}
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

    set (OUTPUT_FOLDER ${CMAKE_CURRENT_BINARY_DIR}/a2x-${DOC_FORMAT})

    if (ASCIIDOC_FOUND AND SOURCE_HIGHLITE_FOUND)
      add_custom_command (
        OUTPUT ${OUTPUT_FOLDER}
        COMMAND mkdir -p ${OUTPUT_FOLDER} &&
                ${A2X_EXECUTABLE}
                -f ${DOC_FORMAT} ${DOC_OPTION}
                --no-xmllint
                --destination-dir=${OUTPUT_FOLDER}
                ${CMAKE_CURRENT_SOURCE_DIR}/${DOC_INPUT} ||
                rm -rf ${OUTPUT_FOLDER}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${DOC_INPUT}
      )
      add_custom_target (${DOC_TARGET}
        ALL
        DEPENDS ${OUTPUT_FOLDER}
      )
    endif()
  endmacro()


  ADD_ASCIIDOC (user-asciidoc-html
    INPUT user.txt
    OUTPUT user.html
    BACKEND xhtml11
    )

  ADD_ASCIIDOC (user-asciidoc-docbook
    INPUT user.txt
    OUTPUT user.docbook
    BACKEND docbook
    )

  ADD_A2X (user-a2x-html
    INPUT user.txt
    FORMAT xhtml
    )

  ADD_A2X (user-a2x-pdf
    INPUT user.txt
    FORMAT pdf
    OPTION --fop
    )
endif()

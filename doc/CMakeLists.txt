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
        COMMAND ${CMAKE_COMMAND}
                -DPATH="${SOURCE_HIGHLITE_BINDIR}:$ENV{PATH}"
                -DASCIIDOC="${ASCIIDOC_EXECUTABLE}"
                -DBACKEND="${DOC_BACKEND}"
                -DOUTPUT="${CMAKE_CURRENT_BINARY_DIR}/${DOC_OUTPUT}"
                -DINPUT="${CMAKE_CURRENT_SOURCE_DIR}/${DOC_INPUT}"
                -P "${CMAKE_SOURCE_DIR}/modules/asciidoc.cmake"
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
        COMMAND ${CMAKE_COMMAND}
                -DA2X="${A2X_EXECUTABLE}"
                -DPATH="${ASCIIDOC_BINDIR}:$ENV{PATH}"
                -DFORMAT="${DOC_FORMAT}"
                -DOPTION="${DOC_OPTION}"
                -DOUTPUT="${OUTPUT_FOLDER}"
                -DINPUT="${CMAKE_CURRENT_SOURCE_DIR}/${DOC_INPUT}"
                -P "${CMAKE_SOURCE_DIR}/modules/a2x.cmake"
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${DOC_INPUT}
      )
      add_custom_target (${DOC_TARGET}
        ALL
        DEPENDS ${OUTPUT_FOLDER}
      )
    endif()
  endmacro()


  ADD_ASCIIDOC (user-asciidoc-html
    INPUT user/gpispace.txt
    OUTPUT user.html
    BACKEND xhtml11
    )

  install (FILES
    ${CMAKE_CURRENT_BINARY_DIR}/user.html
    DESTINATION share/gspc/doc/
  )

  install (FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/user/images/gpi_space_workflow_notext.png
    DESTINATION share/gspc/doc/images
  )

  ADD_ASCIIDOC (user-asciidoc-docbook
    INPUT user/gpispace.txt
    OUTPUT user.docbook
    BACKEND docbook
    )

  ADD_A2X (user-a2x-html
    INPUT user/gpispace.txt
    FORMAT xhtml
    )

  ADD_A2X (user-a2x-pdf
    INPUT user/gpispace.txt
    FORMAT pdf
    OPTION --fop
    )
endif()

add_subdirectory (tutorial)

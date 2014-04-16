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
endif()

add_subdirectory (tutorial)

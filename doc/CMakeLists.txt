if (ENABLE_DOCUMENTATION)
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

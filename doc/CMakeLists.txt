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

add_subdirectory (tutorial)

# bernd.loerwald@itwm.fraunhofer.de

if (ENABLE_DOCUMENTATION)
  ADD_ASCIIDOC (tutorial_sequence-asciidoc-html
    INPUT README
    OUTPUT README.html
    BACKEND xhtml11
  )

  install (FILES
    ${CMAKE_CURRENT_BINARY_DIR}/README.html
    ${CMAKE_CURRENT_BINARY_DIR}/sequence.png
    ${CMAKE_CURRENT_BINARY_DIR}/sequence_two_place_fail.png
    ${CMAKE_CURRENT_BINARY_DIR}/sequence_two_place_good.png
    DESTINATION share/gspc/doc/tutorial/sequence
  )
endif()

install (FILES
  sequence.xpnet
  sequence_forward.xpnet
  use_sequence.xpnet
  README
  DESTINATION share/gspc/doc/tutorial/sequence
)

add_subdirectory (test)

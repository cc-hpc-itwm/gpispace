# mirko.rahn@itwm.fraunhofer.de

ADD_ASCIIDOC (share_example_concurrent-asciidoc-html
  INPUT README
  OUTPUT README.html
  BACKEND xhtml11
)

install (FILES
  ${CMAKE_CURRENT_BINARY_DIR}/README.html
  ${CMAKE_CURRENT_BINARY_DIR}/concurrent.png
  ${CMAKE_CURRENT_BINARY_DIR}/counting.png
  DESTINATION share/gspc/example/concurrent/
)

install (FILES
  concurrent.xpnet
  README
  DESTINATION share/gspc/example/concurrent/
)

add_subdirectory (test)

# bernd.loerwald@itwm.fraunhofer.de

  ADD_ASCIIDOC (share_example_workerlist-asciidoc-html
    INPUT README
    OUTPUT README.html
    BACKEND xhtml11
  )

  install (FILES
    ${CMAKE_CURRENT_BINARY_DIR}/README.html
    DESTINATION share/gspc/example/workerlist
  )

install (FILES README workerlist.xpnet DESTINATION share/gspc/example/workerlist)

add_subdirectory (test)

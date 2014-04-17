# bernd.loerwald@itwm.fraunhofer.de

if (ENABLE_DOCUMENTATION)
  ADD_ASCIIDOC (share_example_workerlist-asciidoc-html
    INPUT README
    OUTPUT README.html
    BACKEND xhtml11
  )

  install (FILES
    ${CMAKE_CURRENT_BINARY_DIR}/README.html
    DESTINATION share/gspc/example/workerlist
  )
endif()

add_subdirectory (test)

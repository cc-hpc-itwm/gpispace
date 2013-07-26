if (WITH_MANPAGES)
  find_package (XmlTo)

  if (XMLTO_FOUND)
    add_custom_command (OUTPUT gspc.1
      COMMAND ${XMLTO_EXECUTABLE} man ${CMAKE_CURRENT_SOURCE_DIR}/gspc.1.xml
      DEPENDS gspc.1.xml)

    add_custom_target (gspc.manpage ALL
      DEPENDS gspc.1)

    install (FILES ${CMAKE_CURRENT_BINARY_DIR}/gspc.1 DESTINATION share/man/man1)
  else()
    message (WARNING "WITH_MANPAGES was set, but xmlto was not found. man-pages will not be installed")
  endif ()
endif ()

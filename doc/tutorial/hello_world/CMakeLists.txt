# mirko.rahn@itwm.fraunhofer.de

if (ENABLE_DOCUMENTATION)
  ADD_ASCIIDOC (tutorial_hello_world-asciidoc-html
    INPUT README
    OUTPUT README.html
    BACKEND xhtml11
  )

  install (FILES
    ${CMAKE_CURRENT_BINARY_DIR}/README.html
    DESTINATION share/gspc/doc/tutorial/hello_world
  )
endif()

install (FILES
  hello_world.xpnet
  Makefile
  hello_many.xpnet
  README
  DESTINATION share/gspc/doc/tutorial/hello_world
)

install (FILES
  include/hello2.h
  include/hello_util.h
  include/hello_world.hpp
  DESTINATION share/gspc/doc/tutorial/hello_world/include
)

install (FILES
  src/hello_world.cpp
  src/hello_util.c
  src/hello2.c
  src/Makefile
  DESTINATION share/gspc/doc/tutorial/hello_world/src
)

add_subdirectory (test)

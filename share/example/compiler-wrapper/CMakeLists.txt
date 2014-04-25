add_subdirectory (test)

ADD_ASCIIDOC (share_example_compiler-wrapper-asciidoc-html
  INPUT README
  OUTPUT README.html
  BACKEND xhtml11
)

install (FILES
  ${CMAKE_CURRENT_BINARY_DIR}/README.html
  DESTINATION share/gspc/example/compiler-wrapper
)

install (FILES Makefile README compile.xpnet
   DESTINATION share/gspc/example/compiler-wrapper
)

install (PROGRAMS bin/sdpacc.in
   DESTINATION share/gspc/example/compiler-wrapper/bin
)

install (FILES src/hello.c
   DESTINATION share/gspc/example/compiler-wrapper/src
)

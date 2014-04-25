add_subdirectory (test)

install (FILES Makefile README compile.xpnet
   DESTINATION share/gspc/example/compiler-wrapper
)

install (PROGRAMS bin/sdpacc.in
   DESTINATION share/gspc/example/compiler-wrapper/bin
)

install (FILES src/hello.c
   DESTINATION share/gspc/example/compiler-wrapper/src
)

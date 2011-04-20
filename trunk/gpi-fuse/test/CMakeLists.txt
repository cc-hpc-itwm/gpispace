include_directories (${CMAKE_CURRENT_SOURCE_DIR})

set (simple_tests
     split
    )

foreach (test ${simple_tests})

  set (name "test_fuse_${test}")
  set (target "test_${test}")
  set (src "${target}.cpp")

  add_executable(${target} ${src})

  target_link_libraries (${target} gpi-fuse)
  target_link_libraries (${target} ${FUSE_LIBRARIES})
  set_target_properties(${target} PROPERTIES
      COMPILE_FLAGS "${PC_FUSE_CFLAGS_OTHER}"
    )

  add_test (${name} ${target})

endforeach (test ${simple_tests})

add_executable (seek seek.c)
set_target_properties (seek PROPERTIES COMPILE_FLAGS -std=c99)

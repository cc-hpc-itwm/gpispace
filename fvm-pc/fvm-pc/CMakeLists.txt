# -*- mode: cmake; -*-
set (hdrs
  assert.hpp
  pc.hpp
  util.hpp
  )

if (ENABLE_DEPRECATED_FVM_PC)
  if (ENABLE_FAKE_PC)
    add_subdirectory(fake)
  endif (ENABLE_FAKE_PC)

#  set (srcs
#    pc.cpp
#    )

  # ADD_LIBRARY(fvm-pc ${srcs})
  # ADD_LIBRARY(fvm-pc.shared SHARED ${srcs})
  # set_target_properties(fvm-pc.shared PROPERTIES OUTPUT_NAME fvm-pc)

install(FILES ${hdrs}
  DESTINATION include/fvm-pc
  COMPONENT runtime-headers
)
#  install(TARGETS fvm-pc
#    ARCHIVE
#    DESTINATION libexec
#    COMPONENT modules)
#  install(TARGETS fvm-pc.shared
#    LIBRARY
#    DESTINATION libexec
#    COMPONENT modules)

endif()

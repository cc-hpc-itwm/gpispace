if (ENABLE_DEPRECATED_FVM_PC)
  if (ENABLE_FAKE_PC)
    add_subdirectory(fake)
  endif (ENABLE_FAKE_PC)

  install(FILES
    pc.hpp
    DESTINATION include/fvm-pc
    COMPONENT runtime-headers
    )
endif()

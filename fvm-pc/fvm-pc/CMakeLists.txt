set (hdrs
  assert.hpp
  pc.hpp
  util.hpp
  )

if (ENABLE_DEPRECATED_FVM_PC)
  if (ENABLE_FAKE_PC)
    add_subdirectory(fake)
  endif (ENABLE_FAKE_PC)

  install(FILES ${hdrs}
    DESTINATION include/fvm-pc
    COMPONENT runtime-headers
    )
endif()

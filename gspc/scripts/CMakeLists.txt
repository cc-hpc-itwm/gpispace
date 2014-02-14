configure_file(gspc-sh-version.in ${CMAKE_CURRENT_BINARY_DIR}/gspc-sh-version @ONLY)

set (prgs
  gspc
)

set (scripts
  gspc-log
  gspc-net

)

set (files
  gspc-sh-setup
  ${CMAKE_CURRENT_BINARY_DIR}/gspc-sh-version
  )

install(PROGRAMS ${prgs}
  DESTINATION bin
  COMPONENT runtime)

install(PROGRAMS ${scripts}
  DESTINATION libexec/gspc/scripts
  COMPONENT runtime)

install (FILES ${files}
  DESTINATION libexec/gspc/scripts
  COMPONENT runtime)

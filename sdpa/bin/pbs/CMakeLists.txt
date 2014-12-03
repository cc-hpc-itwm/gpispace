set (prgs
  sdpa
)

set (scripts
  start-drts
  start-sdpa
  sdpa-selftest
)

install(PROGRAMS ${prgs}
  DESTINATION bin
)

install(PROGRAMS ${scripts}
  DESTINATION libexec/sdpa/scripts
)

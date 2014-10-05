set (prgs
  sdpa
)

set (scripts
  bundle.sh
  start-agent
  start-drts
  start-sdpa
  stop-drts
  stop-agent
  status-agent
  status-drts
  sdpa-selftest
)

install(PROGRAMS ${prgs}
  DESTINATION bin
)

install(PROGRAMS ${scripts}
  DESTINATION libexec/sdpa/scripts
)

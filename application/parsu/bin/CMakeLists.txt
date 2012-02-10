
set (prgs
   parsu_run
)

install(PROGRAMS ${prgs}
  DESTINATION libexec/apps/parsu/bin
  COMPONENT applications)

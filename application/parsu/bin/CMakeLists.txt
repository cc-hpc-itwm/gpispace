
set (prgs
   parsu_run
)

install(PROGRAMS ${prgs}
  DESTINATION ${SDPA_APPS_DIR}/parsu/bin
  COMPONENT applications)

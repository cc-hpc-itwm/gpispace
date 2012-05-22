
set (prgs
   parsu_run
   parsu-sdpa
)

install(PROGRAMS ${prgs}
  DESTINATION ${SDPA_APPS_DIR}/parsu/bin
  COMPONENT applications)

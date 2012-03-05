set (prgs
  sdpacc
  sdpac++
  g++
  c++
  gcc
)

add_subdirectory(xml)

install (PROGRAMS ${prgs}
  DESTINATION "${SDPA_APPS_DIR}/sdpacc/bin"
  COMPONENT applications
)

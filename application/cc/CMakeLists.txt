set (prgs
  sdpacc
  sdpac++
  g++
  c++
  gcc
)

add_subdirectory(xml)

install (PROGRAMS ${prgs}
  DESTINATION libexec/apps/sdpacc/bin
  COMPONENT applications
)

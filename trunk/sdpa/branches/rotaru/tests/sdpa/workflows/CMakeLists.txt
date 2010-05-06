file (GLOB gwdls "*.gwdl")

foreach (wf ${gwdls})
  string(REGEX REPLACE "(.*/)?(.*).gwdl.*" "\\2" wf_name ${wf})
  message(STATUS "adding workflow `${wf_name}' (using ${wf})")
  configure_file(${wf}    ${CMAKE_CURRENT_BINARY_DIR}/${wf_name}.gwdl COPYONLY)
endforeach (wf ${gwdls})

configure_file(simple-net.pnet  ${CMAKE_CURRENT_BINARY_DIR}/simple-net.pnet COPYONLY)
configure_file(kdm.simple.conf  ${CMAKE_CURRENT_BINARY_DIR}/kdm.simple.conf COPYONLY)

install(FILES ${gwdls} DESTINATION share/doc/${PROJECT_NAME}-${V_MAJOR}.${V_MINOR}/workflows)

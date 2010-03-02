file (GLOB gwdls "*.gwdl")

foreach (wf ${gwdls})
  string(REGEX REPLACE "(.*/)?(.*).gwdl.*" "\\2" wf_name ${wf})
  message(STATUS "adding workflow `${wf_name}' (using ${wf})")
  configure_file(${wf}    ${CMAKE_CURRENT_BINARY_DIR}/${wf_name}.gwdl COPYONLY)
endforeach (wf ${gwdls})

install(FILES ${gwdls} DESTINATION share/doc/${PROJECT_NAME}-${V_MAJOR}.${V_MINOR}/workflows)

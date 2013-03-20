set (mods
  CheckCompiler.cmake
  CompilerFlags.cmake
  FindBoost.cmake
  FindCppUnit.cmake
  FindFhgCom.cmake
  FindFhgLog.cmake
  FindFVM.cmake
  FindMMGR.cmake
  FindPackageHelper.cmake
  FindSDPA.cmake
  FindSEDA.cmake
  FindSMC.cmake
  FindUTIL.cmake
  SMCMacros.cmake
  TargetDoc.cmake
  car_cdr_macros.cmake
  cmake_parse_arguments.cmake
  libutils.cmake
  merge_archives_unix.cmake.in
  pnet_macros.cmake
  fhg_plugin_macros.cmake
  fhg_test_macros.cmake
  )

install(FILES ${mods}
  DESTINATION share/sdpa/cmake
  COMPONENT cmake-scripts)

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
  FindUUID.cmake
  FindWE.cmake
  FindXML.cmake
  SMCMacros.cmake
  TargetDoc.cmake
  )

install(FILES ${mods}
  DESTINATION share/sdpa/cmake
  COMPONENT libraries)

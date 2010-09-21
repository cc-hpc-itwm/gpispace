set (hdrs
  )

set (srcs
  fvmAllocator.c
  fvmConfig.c
  fvmLogger.c
  fvm.c
  fvmSync.c
  )

add_library (sdpa-gpi-lib ${srcs})
add_executable (sdpa-gpi fvm_standalone.c)
target_link_libraries (sdpa-gpi
  sdpa-gpi-lib
  sdpa-gpi-datastructures
  ${EXT_LIBRARIES})


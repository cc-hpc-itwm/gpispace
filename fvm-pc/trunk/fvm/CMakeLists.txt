# -*- mode: cmake; -*-
set (hdrs
  fvmAllocatorTypes.h
  fvm_common.h
  )

install(FILES ${hdrs}
  DESTINATION include/fvm
  COMPONENT headers)

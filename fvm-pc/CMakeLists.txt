project(libfvm-pc)

include_directories (${CMAKE_CURRENT_SOURCE_DIR})

add_definitions (-DSHMEM)

  if (MAX_SHMEM_SIZE)
  else()
    set (MAX_SHMEM_SIZE 536870912) # 512 MB
  endif()
  if (MAX_FVM_SIZE)
  else()
    set (MAX_FVM_SIZE 536870912) # 512 MB
  endif()
  add_definitions (-DMAX_SHMEM_SIZE=${MAX_SHMEM_SIZE})
  add_definitions (-DMAX_FVM_SIZE=${MAX_FVM_SIZE})

include_directories (${CMAKE_SOURCE_DIR}/gspc)
include_directories (${CMAKE_SOURCE_DIR}/src)

include_directories (${FhgLog_INCLUDE_DIR})

include_directories(${UTIL_INCLUDE_DIR})

add_subdirectory (fvm-pc)

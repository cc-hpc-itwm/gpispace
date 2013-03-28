project(libfvm-pc)

option (ENABLE_DEPRECATED_FVM_PC "Build the (deprecated) fvm pc library" ON)
option (ENABLE_FAKE_PC "Build with fake virtual machine interface library" ON)
set (FVM_FVM_SIZE "3221225472" CACHE STRING "Default virtual machine size")
set (FVM_SHM_SIZE "3221225472" CACHE STRING "Default shared memory segment size")
set (FVM_FAKE_FVM_SIZE "52428800" CACHE STRING "Default fake virtual machine size")
set (FVM_FAKE_SHM_SIZE "52428800" CACHE STRING "Default fake shared memory segment size")
set (FVM_MSQ_FILE "${CMAKE_INSTALL_PREFIX}/etc/gpi/key"  CACHE STRING "Default MSQ file")
set (FVM_SHM_FILE "${CMAKE_INSTALL_PREFIX}/etc/gpi/shm" CACHE STRING "Default SHM file")

message (STATUS "FVM config: shmsize=${FVM_SHM_SIZE} fvmsize=${FVM_FVM_SIZE}")

include (CheckCompiler)
if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  include (CompilerFlags)
endif()

include_directories (${${PROJECT_NAME}_SOURCE_DIR})

link_directories (${CMAKE_LIBRARY_PATH})
link_directories (${${PROJECT_NAME}_BINARY_DIR})

add_definitions (-DSHMEM -DNDEBUGALLOC -DNDEBUG -DNDEBUGCOMM -DNDEBUGMSG)

if (ENABLE_FAKE_PC)
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
endif()

include (SDPAFindBoost)
include_directories (SYSTEM ${Boost_INCLUDE_DIRS})

include_directories (${CMAKE_SOURCE_DIR}/pnet/src)

find_package (FhgLog REQUIRED)
include_directories (${FhgLog_INCLUDE_DIR})

find_package (UTIL REQUIRED)
include_directories(${UTIL_INCLUDE_DIR})

add_subdirectory (fvm)
add_subdirectory (fvm-pc)

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/etc/fvm.cfg.in ${CMAKE_CURRENT_BINARY_DIR}/etc/fvm.cfg @ONLY)
configure_file (${CMAKE_CURRENT_SOURCE_DIR}/etc/fvm.cfg.in ${CMAKE_CURRENT_BINARY_DIR}/etc/fvm.fake.cfg @ONLY)

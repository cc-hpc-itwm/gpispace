find_path (FVM_INCLUDE_DIR
  NAMES "fvm-pc/pc.hpp"
  HINTS ${FVM_HOME} ENV FVM_HOME
  PATH_SUFFIXES include
)

if (FVM_INCLUDE_DIR)
  include_directories(${FVM_INCLUDE_DIR})
else (FVM_INCLUDE_DIR)
  message(FATAL_ERROR "KDM modules requires the FVM process container header")
endif(FVM_INCLUDE_DIR)

add_library(kdm SHARED kdm.cpp)
install(TARGETS kdm DESTINATION lib)

add_subdirectory (fake)

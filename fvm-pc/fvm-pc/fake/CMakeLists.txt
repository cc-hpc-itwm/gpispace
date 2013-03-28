# -*- mode: cmake; -*-
find_package (MMGR REQUIRED QUIET)
include_directories (${MMGR_INCLUDE_DIR})

set (pc-srcs pc.cpp)

ADD_LIBRARY(fvm-pc-fake ${pc-srcs})
ADD_LIBRARY(fvm-pc-fake.shared SHARED ${pc-srcs})
target_link_libraries(fvm-pc-fake ${MMGR_LIBRARY})
target_link_libraries(fvm-pc-fake.shared ${MMGR_LIBRARY_SHARED})
set_target_properties(fvm-pc-fake PROPERTIES OUTPUT_NAME fvm-pc)
set_target_properties(fvm-pc-fake.shared PROPERTIES OUTPUT_NAME fvm-pc)

#install(TARGETS fvm-pc-fake
#  ARCHIVE
#  DESTINATION libexec/sdpa/fake
#  COMPONENT modules)
install(TARGETS fvm-pc-fake.shared
  LIBRARY
  DESTINATION libexec/sdpa/fake
  COMPONENT modules)

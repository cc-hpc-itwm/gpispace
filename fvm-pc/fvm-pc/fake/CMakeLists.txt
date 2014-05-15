set (pc-srcs pc.cpp)

ADD_LIBRARY(fvm-pc-fake.shared SHARED ${pc-srcs})
target_link_libraries(fvm-pc-fake.shared ${MMGR_LIBRARY_SHARED} pnet)
set_target_properties(fvm-pc-fake.shared PROPERTIES OUTPUT_NAME fvm-pc)

install(TARGETS fvm-pc-fake.shared
  LIBRARY
  DESTINATION libexec/sdpa/fake
)

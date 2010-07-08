find_package(FVM REQUIRED)
include_directories(${FVM_INCLUDE_DIR})

add_library(kdm SHARED kdm.cpp)
install(TARGETS kdm DESTINATION lib)

add_library(kdm_complex SHARED kdm_complex.cpp)
install(TARGETS kdm_complex DESTINATION lib)

add_subdirectory (fake)

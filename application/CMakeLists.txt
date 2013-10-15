set (PNETC_LIB_DIR "share/sdpa/xml/lib")
set (PNETC_EXAMPLES_DIR "share/sdpa/xml/examples")
set (SDPA_APPS_DIR "libexec/sdpa/apps")

find_package(UTIL REQUIRED QUIET)
include_directories(${UTIL_INCLUDE_DIR})

add_subdirectory(lib)
add_subdirectory(example)
add_subdirectory(kdm)
add_subdirectory(parsu)
add_subdirectory(asian)
add_subdirectory(cc)
add_subdirectory(selftest)
add_subdirectory(transform_file)
add_subdirectory(mapreduce)

option(ENABLE_APPLICATION_UFBMIG "Compile UfbMig Application" ON)
if (ENABLE_APPLICATION_UFBMIG)
   add_subdirectory(ufbmig)
endif()

install ( FILES common.mk
          DESTINATION share/sdpa/make
          COMPONENT sdk
        )

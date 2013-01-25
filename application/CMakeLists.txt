set (PNETC_LIB_DIR "share/sdpa/xml/lib")
set (PNETC_EXAMPLES_DIR "share/sdpa/xml/examples")
set (SDPA_APPS_DIR "libexec/sdpa/apps")

add_subdirectory(lib)
add_subdirectory(example)
add_subdirectory(kdm)
add_subdirectory(parsu)
add_subdirectory(asian)
add_subdirectory(cc)
add_subdirectory(selftest)
add_subdirectory(transform_file)
add_subdirectory(mapreduce)

option(WITH_APPLICATION_UFBMIG "Compile UfbMig Application" OFF)
if (WITH_APPLICATION_UFBMIG)
   add_subdirectory(ufbmig)
endif()

install ( FILES common.mk
          DESTINATION share/sdpa/make
          COMPONENT sdk
        )

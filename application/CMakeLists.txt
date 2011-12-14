set (PNETC_LIB_DIR "share/sdpa/xml/lib")

add_subdirectory(lib)
add_subdirectory(example)
add_subdirectory(kdm)
add_subdirectory(parsu)
add_subdirectory(asian)
add_subdirectory(cc)
add_subdirectory(selftest)

option(WITH_APPLICATION_UFBMIG "Compile UfbMig Application" OFF)
if (WITH_APPLICATION_UFBMIG)
   add_subdirectory(ufbmig)
endif()

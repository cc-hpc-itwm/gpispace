configure_file(assert_config.hpp.in assert_config.hpp @ONLY)

add_subdirectory(util)
add_subdirectory(plugin)

execute_process(COMMAND hostname -f
                OUTPUT_VARIABLE FHG_BUILD_HOST
                OUTPUT_STRIP_TRAILING_WHITESPACE
               )
execute_process(COMMAND id -un
                OUTPUT_VARIABLE FHG_BUILD_USER
                OUTPUT_STRIP_TRAILING_WHITESPACE
               )
execute_process(COMMAND uname -p
                OUTPUT_VARIABLE FHG_BUILD_CPU
                OUTPUT_STRIP_TRAILING_WHITESPACE
               )
execute_process(COMMAND uname -s
                OUTPUT_VARIABLE FHG_BUILD_KERNEL
                OUTPUT_STRIP_TRAILING_WHITESPACE
               )
execute_process(COMMAND uname -v
                OUTPUT_VARIABLE FHG_BUILD_KERNEL_VERSION
                OUTPUT_STRIP_TRAILING_WHITESPACE
               )
execute_process(COMMAND date
                OUTPUT_VARIABLE FHG_BUILD_TIMESTAMP
                OUTPUT_STRIP_TRAILING_WHITESPACE
               )
execute_process(COMMAND ${CMAKE_CXX_COMPILER} --version
                COMMAND head -n 1
                OUTPUT_VARIABLE FHG_BUILD_COMPILER
                OUTPUT_STRIP_TRAILING_WHITESPACE
               )
set(FHG_BUILD_INFO "${FHG_BUILD_KERNEL} ${FHG_BUILDL_CPU} ${FHG_BUILD_USER}@${FHG_BUILD_HOST} Build #${PROJECT_BUILD}")
set(FHG_BUILD_COUNT ${PROJECT_BUILD})

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/revision.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/revision.cpp @ONLY)

add_library (fhg-revision ${CMAKE_CURRENT_BINARY_DIR}/revision.cpp)

install (FILES revision.hpp
               assert.hpp
               assert_modes.hpp
               assertion_failed.hpp
               error_codes.hpp
  DESTINATION include/fhg
  COMPONENT headers
  )

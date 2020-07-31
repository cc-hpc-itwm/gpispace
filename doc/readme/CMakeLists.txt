# Add a target for \a output_file which is produced by replacing
# `@@part@@` in \a input_file with parts in \a parts_dir.
function (add_file_assembled_from_parts output_file input_file)
  set (_parts_dir "${CMAKE_CURRENT_SOURCE_DIR}/parts/")

  # \todo This lets every output depend on all parts, even if not
  # used. Instead, one should parse the files instead to determine
  # dependencies, but this seems to be impossible to nicely do with
  # cmake.
  file (GLOB all_parts
    LIST_DIRECTORIES false
    CONFIGURE_DEPENDS
    "${_parts_dir}/*"
  )

  add_custom_command (OUTPUT "${output_file}"
    COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/configure.sh"
            "${_parts_dir}"
            "${input_file}"
            "${output_file}"
    MAIN_DEPENDENCY "${input_file}"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/configure.sh"
            ${all_parts}
  )

  string (MAKE_C_IDENTIFIER "${output_file}" target_name)
  add_custom_target ("${target_name}"
    ALL
    DEPENDS "${output_file}"
  )
endfunction()

add_file_assembled_from_parts ("${CMAKE_CURRENT_BINARY_DIR}/getting_started.md"
  "${CMAKE_CURRENT_SOURCE_DIR}/parts/readme-full"
)
add_file_assembled_from_parts ("${CMAKE_CURRENT_BINARY_DIR}/driver.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/parts/driver-full"
)
add_file_assembled_from_parts ("${CMAKE_CURRENT_BINARY_DIR}/build.sh"
  "${CMAKE_CURRENT_SOURCE_DIR}/parts/script-full"
)
add_file_assembled_from_parts ("${CMAKE_CURRENT_BINARY_DIR}/compute_and_aggregate.xpnet"
  "${CMAKE_CURRENT_SOURCE_DIR}/parts/pnet-full"
)

install (FILES "${CMAKE_CURRENT_BINARY_DIR}/getting_started.md"
  DESTINATION "share/gspc/"
)
install (FILES "${CMAKE_CURRENT_SOURCE_DIR}/img/GPISpace_arch.png"
               "${CMAKE_CURRENT_SOURCE_DIR}/img/gpispace_example_pnet.png"
               "${CMAKE_CURRENT_SOURCE_DIR}/img/petri_net.png"
  DESTINATION "share/gspc/img/"
)
install (FILES "${CMAKE_CURRENT_BINARY_DIR}/compute_and_aggregate.xpnet"
               "${CMAKE_CURRENT_BINARY_DIR}/driver.cpp"
  DESTINATION "share/gspc/example-compute_and_aggregate/"
)
install (PROGRAMS "${CMAKE_CURRENT_BINARY_DIR}/build.sh"
  DESTINATION "share/gspc/example-compute_and_aggregate/"
)

# \todo `fhg_add_test()` requires SOURCES. Factor out the common part...
add_test (NAME doc-readme-getting_started-build
  COMMAND "${CMAKE_INSTALL_PREFIX}/share/gspc/example-compute_and_aggregate/build.sh"
          "${CMAKE_INSTALL_PREFIX}"
          "${CMAKE_CURRENT_BINARY_DIR}/doc-readme-getting_started-build"
          "" # don't start monitor
          "${CMAKE_CXX_COMPILER}"
)
set_property (TEST doc-readme-getting_started-build
  APPEND PROPERTY LABELS "requires_installation"
)

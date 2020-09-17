install (FILES "${CMAKE_CURRENT_SOURCE_DIR}/getting_started.md"
  DESTINATION "share/gspc/"
)
install (FILES "${CMAKE_CURRENT_SOURCE_DIR}/img/GPISpace_arch.svg"
               "${CMAKE_CURRENT_SOURCE_DIR}/img/gpispace_example_pnet.svg"
               "${CMAKE_CURRENT_SOURCE_DIR}/img/petri_net.svg"
  DESTINATION "share/gspc/img/"
)
install (FILES "${CMAKE_CURRENT_SOURCE_DIR}/compute_and_aggregate.xpnet"
               "${CMAKE_CURRENT_SOURCE_DIR}/driver.cpp"
  DESTINATION "share/gspc/example-compute_and_aggregate/"
)
install (PROGRAMS "${CMAKE_CURRENT_SOURCE_DIR}/build.sh"
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

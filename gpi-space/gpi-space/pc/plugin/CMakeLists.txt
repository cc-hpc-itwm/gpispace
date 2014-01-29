include_directories(${FVM_INCLUDE_DIR})

include(fhg_plugin_macros)

fhg_add_plugin(gpi gpi.cpp
  LINK_LIBRARIES
    gpi-space-pc-client-static
    gpi-space-pc-segment
    gpi-space-static
    ${FhgLog_LIBRARY_SHARED}
    ${Boost_SERIALIZATION_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
  INSTALL
  HEADERS gpi.hpp
)

fhg_add_plugin(gpi_compat gpi_compat.cpp
  LINK_LIBRARIES
    ${Boost_SERIALIZATION_LIBRARY}
    fhg-util
  INSTALL
)

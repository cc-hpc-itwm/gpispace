###############################################################################
# IML - part of GPI-Space (not a standalone project)
###############################################################################

include (parse_arguments)

find_package (GASPI 1.5 REQUIRED)

add_library (BeeGFS-ClientDevel INTERFACE)
add_library (BeeGFS::ClientDevel ALIAS BeeGFS-ClientDevel)
target_include_directories (BeeGFS-ClientDevel
  SYSTEM INTERFACE
    "${PROJECT_SOURCE_DIR}/3rdparty/beegfs-client-devel"
)

###############################################################################
# Definitions
###############################################################################

set (iml_devel_files_in_installation
  "include/gspc/iml/AllocationHandle.hpp"
  "include/gspc/iml/AllocationHandle.ipp"
  "include/gspc/iml/Client.hpp"
  "include/gspc/iml/MemcpyID.hpp"
  "include/gspc/iml/MemoryLocation.hpp"
  "include/gspc/iml/MemoryLocation.ipp"
  "include/gspc/iml/MemoryOffset.hpp"
  "include/gspc/iml/MemoryRegion.hpp"
  "include/gspc/iml/MemoryRegion.ipp"
  "include/gspc/iml/MemorySize.hpp"
  "include/gspc/iml/Rifs.hpp"
  "include/gspc/iml/RuntimeSystem.hpp"
  "include/gspc/iml/SegmentAndAllocation.hpp"
  "include/gspc/iml/SegmentDescription.hpp"
  "include/gspc/iml/SegmentHandle.hpp"
  "include/gspc/iml/SegmentHandle.ipp"
  "include/gspc/iml/SharedMemoryAllocation.hpp"
  "include/gspc/iml/SharedMemoryAllocationHandle.hpp"
  "include/gspc/iml/SharedMemoryAllocationHandle.ipp"
  "include/gspc/iml/beegfs/SegmentDescription.hpp"
  "include/gspc/iml/beegfs/SegmentDescription.ipp"
  "include/gspc/iml/gaspi/NetdevID.hpp"
  "include/gspc/iml/gaspi/NetdevID.ipp"
  "include/gspc/iml/gaspi/SegmentDescription.hpp"
  "include/gspc/iml/gaspi/SegmentDescription.ipp"
  "include/gspc/iml/rif/EntryPoints.hpp"
)

set (iml_files_in_installation
  "${GSPC_LIBRARY_INSTALL_DIR}/libgspc_iml_client.so"
  "libexec/iml/gspc-iml-gpi-server"
  "libexec/iml/libgspc_iml_installation.so"
  ${iml_devel_files_in_installation}
)

###############################################################################
# Project targets
###############################################################################

gspc_add_library (NAME gspc_iml_headers
  TYPE INTERFACE
  INCLUDE_DIRECTORIES
    INTERFACE
      $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    INTERFACE
      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_compile_features (gspc_iml_headers
  INTERFACE cxx_std_17
)

###############################################################################
# IML libraries and executables
###############################################################################

gspc_add_library (
  NAME gspc_iml_shared
  TYPE STATIC
  SOURCES "vmem/gaspi/pc/segment/segment.cpp"
  LIBRARIES
    PUBLIC gspc_iml_headers
    PRIVATE gspc_util
    PUBLIC Boost::base
    PRIVATE Boost::system
  INCLUDE_DIRECTORIES
    PUBLIC "${PROJECT_SOURCE_DIR}/include"
  POSITION_INDEPENDENT
)

gspc_add_library (
  NAME gspc_iml_client
  TYPE SHARED
  SOURCES "AllocationHandle.cpp"
          "Client.cpp"
          "MemoryLocation.cpp"
          "MemoryRegion.cpp"
          "Rifs.cpp"
          "RuntimeSystem.cpp"
          "SegmentAndAllocation.cpp"
          "SegmentHandle.cpp"
          "SharedMemoryAllocation.cpp"
          "SharedMemoryAllocationHandle.cpp"
          "beegfs/SegmentDescription.cpp"
          "gaspi/NetdevID.cpp"
          "gaspi/SegmentDescription.cpp"
          "detail/startup_and_shutdown.cpp"
  LIBRARIES
    PRIVATE gspc_iml_headers
    PRIVATE gspc_iml_installation
    PRIVATE gspc_iml_shared
    PRIVATE gspc_rpc
    PRIVATE gspc_rif_entry_point
    PRIVATE gspc_rif_strategies
    PRIVATE gspc_logging
    PRIVATE gspc_util
    PUBLIC Boost::program_options
    PUBLIC Boost::serialization
    PRIVATE Boost::thread
    PUBLIC fmt
  INCLUDE_DIRECTORIES
    PUBLIC "${PROJECT_SOURCE_DIR}/include"
  INSTALL
    INSTALL_DESTINATION "${GSPC_LIBRARY_INSTALL_DIR}"
    CREATE_BUNDLE_INFO
    RPATH "$ORIGIN/../libexec/iml"
)

gspc_add_library (
  NAME gspc_iml_server
  TYPE STATIC
  SOURCES "vmem/dtmmgr.cpp"
          "vmem/gaspi/gpi/gaspi.cpp"
          "vmem/gaspi/pc/container/manager.cpp"
          "vmem/gaspi/pc/global/topology.cpp"
          "vmem/gaspi/pc/memory/beegfs_area.cpp"
          "vmem/gaspi/pc/memory/gaspi_area.cpp"
          "vmem/gaspi/pc/memory/handle_generator.cpp"
          "vmem/gaspi/pc/memory/manager.cpp"
          "vmem/gaspi/pc/memory/memory_area.cpp"
          "vmem/gaspi/pc/memory/shm_area.cpp"
          "vmem/gaspi_context.cpp"
          "vmem/segment/beegfs.cpp"
          "vmem/tmmgr.cpp"
  LIBRARIES
    PUBLIC gspc_iml_client
    PUBLIC gspc_iml_headers
    PUBLIC gspc_iml_shared
    PUBLIC gspc_rpc
    PRIVATE gspc_util
    PRIVATE BeeGFS::ClientDevel
    PRIVATE GASPI::static
    PUBLIC Boost::serialization
    PRIVATE Boost::system
    PUBLIC Boost::thread
    PRIVATE fmt
)

gspc_add_executable (
  NAME gspc-iml-gpi-server
  RUNTIME
  SOURCES "server/iml-gpi-server.cpp"
  LIBRARIES gspc_iml_headers
            gspc_iml_server
            gspc_rif_started_process_promise
            gspc_util
            Boost::program_options
            Boost::serialization
            Boost::thread
            Boost::system
  INSTALL_DESTINATION "libexec/iml"
  CREATE_BUNDLE_INFO
  RPATH "$ORIGIN/../../${GSPC_LIBRARY_INSTALL_DIR}"
)

gspc_add_library (
  NAME gspc_iml_installation
  TYPE SHARED
  SOURCES "detail/Installation.cpp"
  LIBRARIES
    PUBLIC gspc_iml_headers
    PRIVATE gspc_util
  COMPILE_DEFINITIONS
    PRIVATE
      "-DIML_INSTALLATION_SENTINEL_SUBDIR_COUNT=3"
  INSTALL
    INSTALL_DESTINATION "libexec/iml"
    CREATE_BUNDLE_INFO
)

###############################################################################
# Install IML headers
###############################################################################

foreach (_header ${iml_devel_files_in_installation})
  get_filename_component (_dir "${_header}" DIRECTORY)
  install (
    FILES
      "${PROJECT_SOURCE_DIR}/${_header}"
    DESTINATION
      "${_dir}"
  )
endforeach()

install (
  FILES
    "${PROJECT_SOURCE_DIR}/include/gspc/drts/certificates.hpp"
    "${PROJECT_SOURCE_DIR}/include/gspc/drts/client.fwd.hpp"
    "${PROJECT_SOURCE_DIR}/include/gspc/drts/client.hpp"
    "${PROJECT_SOURCE_DIR}/include/gspc/drts/drts.fwd.hpp"
    "${PROJECT_SOURCE_DIR}/include/gspc/drts/drts.hpp"
    "${PROJECT_SOURCE_DIR}/include/gspc/drts/information_to_reattach.fwd.hpp"
    "${PROJECT_SOURCE_DIR}/include/gspc/drts/information_to_reattach.hpp"
    "${PROJECT_SOURCE_DIR}/include/gspc/drts/pimpl.hpp"
    "${PROJECT_SOURCE_DIR}/include/gspc/drts/rifd_entry_points.fwd.hpp"
    "${PROJECT_SOURCE_DIR}/include/gspc/drts/rifd_entry_points.hpp"
    "${PROJECT_SOURCE_DIR}/include/gspc/drts/scoped_rifd.fwd.hpp"
    "${PROJECT_SOURCE_DIR}/include/gspc/drts/scoped_rifd.hpp"
    "${PROJECT_SOURCE_DIR}/include/gspc/drts/stream.hpp"
    "${PROJECT_SOURCE_DIR}/include/gspc/drts/virtual_memory.hpp"
    "${PROJECT_SOURCE_DIR}/include/gspc/drts/worker_description.hpp"
  DESTINATION
    include/gspc/drts/
)

if (GSPC_WITH_IML)
  install (
    FILES
      "${PROJECT_SOURCE_DIR}/include/gspc/drts/drts_iml.hpp"
      "${PROJECT_SOURCE_DIR}/include/gspc/drts/stream.fwd.hpp"
      "${PROJECT_SOURCE_DIR}/include/gspc/drts/StreamSlot.hpp"
      "${PROJECT_SOURCE_DIR}/include/gspc/drts/virtual_memory.fwd.hpp"
    DESTINATION
      include/gspc/drts/
  )
endif()

install (
  FILES
    "${PROJECT_SOURCE_DIR}/include/gspc/drts/worker/context.hpp"
    "${PROJECT_SOURCE_DIR}/include/gspc/drts/worker/context_fwd.hpp"
  DESTINATION
    include/gspc/drts/worker/
)

gspc_add_library (NAME gspc
  TYPE SHARED
  SOURCES client.cpp
          drts.cpp
          "$<$<BOOL:${GSPC_WITH_IML}>:drts_iml.cpp>"
          information_to_reattach.cpp
          private/option.cpp
          private/startup_and_shutdown.cpp
          rifd_entry_points.cpp
          scoped_rifd.cpp
          "$<$<BOOL:${GSPC_WITH_IML}>:stream.cpp>"
          "$<$<BOOL:${GSPC_WITH_IML}>:virtual_memory.cpp>"
          worker_description.cpp
  INCLUDE_DIRECTORIES
    PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    PUBLIC $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  LIBRARIES GPISpace::header-only
            gspc_scheduler
            gspc_rif_strategies
            gspc_configuration
            gspc_util
            Boost::program_options
            $<$<BOOL:${GSPC_WITH_IML}>:gspc_iml_client>
            # The following two are intended to be merged into
            # libgspc to provide public API.
            gspc_xml_parse
            gspc_rif_started_process_promise
            fmt
  INSTALL
  CREATE_BUNDLE_INFO
)

gspc_add_library (NAME scheduler-types
  TYPE STATIC
  SOURCES scheduler_types.cpp
  INCLUDE_DIRECTORIES
    PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    PUBLIC $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  LIBRARIES GPISpace::header-only
            gspc_util
)

gspc_add_executable (NAME gspc-drts-kernel
  RUNTIME
  # \note Exporting symbols so that modules see e.g. API guard
  # symbol and don't need to link explicitly against pnet or we.
  ENABLE_EXPORTS
  SOURCES worker/drts-kernel.cpp
          worker/drts.cpp
  INCLUDE_DIRECTORIES
    PRIVATE "${PROJECT_SOURCE_DIR}/include"
  LIBRARIES GPISpace::api_guard
            GPISpace::header-only
            gspc_util
            fhgcom
            gspc_scheduler
            gspc_we
            $<$<BOOL:${GSPC_WITH_IML}>:gspc_iml_client>
            gspc_rif_started_process_promise
            gspc_util
            Boost::program_options
            Boost::serialization
            Boost::system
            Boost::thread
            HWLOC::static
  CREATE_BUNDLE_INFO
  INSTALL_DESTINATION "libexec/gspc"
)

gspc_add_library (NAME gspc_worker_context
  TYPE SHARED
  SOURCES worker/context.cpp
  INCLUDE_DIRECTORIES
    PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    PUBLIC $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  LIBRARIES GPISpace::header-only
            gspc_logging
            Boost::iostreams
            fmt
  INSTALL
  CREATE_BUNDLE_INFO
)

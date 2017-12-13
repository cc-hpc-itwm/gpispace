extended_add_library (NAME gpi-space-gpiapi
  SOURCES "gpi/system.cpp"
          "gpi/gaspi.cpp"
  LIBRARIES GASPI::static
            Util::Generic
            GPISpace::vmem
  INCLUDE_DIRECTORIES PRIVATE "${GASPI_INCLUDE_DIR}"
  SYSTEM_INCLUDE_DIRECTORIES PRIVATE "${LIBRT_INCLUDE_DIR}"
)

extended_add_library (NAME gpi-space-pc-client-static
  SOURCES "pc/client/api.cpp"
  LIBRARIES Util::Generic
  SYSTEM_INCLUDE_DIRECTORIES PRIVATE "${LIBRT_INCLUDE_DIR}"
)

extended_add_library (NAME gpi-space-pc-segment
  SOURCES "pc/segment/segment.cpp"
  LIBRARIES "${LIBRT_SHARED_LIBRARY}" Util::Generic
  SYSTEM_INCLUDE_DIRECTORIES PRIVATE "${LIBRT_INCLUDE_DIR}"
)

extended_add_library (NAME gpi-space-pc-memory
  SOURCES "pc/memory/memory_area.cpp"
          "pc/memory/handle_generator.cpp"
          "pc/memory/manager.cpp"
          "pc/memory/gaspi_area.cpp"
          "pc/memory/shm_area.cpp"
          "pc/memory/beegfs_area.cpp"
  LIBRARIES gpi-space-pc-segment
            mmgr
            gpi-space_url
            Util::Generic
            gpi-space-gpiapi
  SYSTEM_INCLUDE_DIRECTORIES PRIVATE "${LIBRT_INCLUDE_DIR}"
)

extended_add_library (NAME gpi-space-pc-container
  SOURCES "pc/container/manager.cpp"
  LIBRARIES Util::Generic
  SYSTEM_INCLUDE_DIRECTORIES PRIVATE "${LIBRT_INCLUDE_DIR}"
)

extended_add_library (NAME gpi-space-pc-global
  SOURCES "pc/global/topology.cpp"
  LIBRARIES Util::Generic RPC
  SYSTEM_INCLUDE_DIRECTORIES PRIVATE "${LIBRT_INCLUDE_DIR}"
)

extended_add_library (NAME gpi-space_url
  SOURCES "pc/url.cpp"
  LIBRARIES fhg-util
)

extended_add_library (NAME gpi_type_check
  SOURCES "types.cpp"
  LIBRARIES gpi-space-gpiapi
)

add_subdirectory (pc/test)
add_subdirectory (tests)

install (FILES
  pc/type/types.hpp
  pc/type/handle.hpp
  pc/type/memory_location.hpp
  DESTINATION include/iml/vmem/gaspi/pc/type/
)

install (FILES
  types.hpp
  DESTINATION include/iml/vmem/gaspi/
)

extended_add_library (NAME vmem-gpiapi
  NAMESPACE IML
  SOURCES "gpi/system.cpp"
          "gpi/gaspi.cpp"
  LIBRARIES GASPI::static
            Util::Generic
            IML::vmem-client
  SYSTEM_INCLUDE_DIRECTORIES PRIVATE "${LIBRT_INCLUDE_DIR}"
)

extended_add_library (NAME vmem-pc-client-static
  NAMESPACE IML
  SOURCES "pc/client/api.cpp"
  LIBRARIES Util::Generic
  SYSTEM_INCLUDE_DIRECTORIES PRIVATE "${LIBRT_INCLUDE_DIR}"
)

extended_add_library (NAME vmem-pc-segment
  NAMESPACE IML
  SOURCES "pc/segment/segment.cpp"
  LIBRARIES "${LIBRT_SHARED_LIBRARY}"
            Util::Generic
  SYSTEM_INCLUDE_DIRECTORIES PRIVATE "${LIBRT_INCLUDE_DIR}"
)

extended_add_library (NAME vmem-pc-memory
  NAMESPACE IML
  SOURCES "pc/memory/memory_area.cpp"
          "pc/memory/handle_generator.cpp"
          "pc/memory/manager.cpp"
          "pc/memory/gaspi_area.cpp"
          "pc/memory/shm_area.cpp"
          "pc/memory/beegfs_area.cpp"
  LIBRARIES IML::vmem-pc-segment
            IML::mmgr
            IML::vmem_url
            Util::Generic
            IML::vmem-gpiapi
  SYSTEM_INCLUDE_DIRECTORIES PRIVATE "${LIBRT_INCLUDE_DIR}"
)

extended_add_library (NAME vmem-pc-container
  NAMESPACE IML
  SOURCES "pc/container/manager.cpp"
  LIBRARIES Util::Generic
  SYSTEM_INCLUDE_DIRECTORIES PRIVATE "${LIBRT_INCLUDE_DIR}"
)

extended_add_library (NAME vmem-pc-global
  NAMESPACE IML
  SOURCES "pc/global/topology.cpp"
  LIBRARIES Util::Generic RPC
  SYSTEM_INCLUDE_DIRECTORIES PRIVATE "${LIBRT_INCLUDE_DIR}"
)

extended_add_library (NAME vmem_url
  NAMESPACE IML
  SOURCES "pc/url.cpp"
  LIBRARIES IML::fhg-util
)

extended_add_library (NAME gpi_type_check
  NAMESPACE IML
  SOURCES "types.cpp"
  LIBRARIES IML::vmem-gpiapi
)

add_subdirectory (pc/test)
add_subdirectory (tests)

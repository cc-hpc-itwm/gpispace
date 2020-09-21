install (FILES
  netdev_id.hpp
  netdev_id.ipp
  DESTINATION include/iml/vmem/
)

extended_add_library (NAME vmem-client
  NAMESPACE IML
  SOURCES "gaspi_context.cpp"
          "netdev_id.cpp"
          "segment/beegfs.cpp"
  LIBRARIES GASPI::static
            BeeGFS::ClientDevel
            Util::Generic
            Boost::program_options
)

extended_add_library (NAME mmgr
  NAMESPACE IML
  SOURCES "dtmmgr.cpp"
          "tmmgr.cpp"
  LIBRARIES Boost::base
)

add_subdirectory (gaspi)
add_subdirectory (tests)

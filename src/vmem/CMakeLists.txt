extended_add_library (NAME vmem
  NAMESPACE GPISpace
  SOURCES "gaspi_context.cpp"
          "netdev_id.cpp"
          "segment/beegfs.cpp"
  LIBRARIES GASPI::static
            BeeGFS::ClientDevel
            Util::Generic
            Boost::program_options
)

fhg_add_test (NAME vmem_netdev_id
  SOURCES "test/netdev_id.cpp"
  LIBRARIES GPISpace::vmem
  USE_BOOST
)

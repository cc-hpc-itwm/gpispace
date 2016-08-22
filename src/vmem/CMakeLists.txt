extended_add_library (NAME vmem
  NAMESPACE GPISpace
  SOURCES "gaspi_context.cpp"
          "segment/beegfs.cpp"
  LIBRARIES GASPI::static
            BeeGFS::ClientDevel
            Util::Generic
)

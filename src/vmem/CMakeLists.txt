extended_add_library (NAME vmem
  NAMESPACE GPISpace
  SOURCES "gaspi_context.cpp"
          "segment/beegfs.cpp"
  LIBRARIES GASPI::static
            BeeGFS::ClientDevel
            Util::Generic
)
#! \todo move into extended_add_library
target_include_directories (GPISpace-vmem
  SYSTEM PRIVATE "${GASPI_INCLUDE_DIR}"
)

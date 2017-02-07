extended_add_library (NAME vmem
  NAMESPACE GPISpace
  SOURCES "segment/beegfs.cpp"
  LIBRARIES BeeGFS::ClientDevel
            Util::Generic
)

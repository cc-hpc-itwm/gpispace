install (FILES
  iml.fwd.hpp
  iml.hpp
  iml.pimpl.hpp
  rifd_entry_points.fwd.hpp
  rifd_entry_points.hpp
  scoped_rifd.fwd.hpp
  scoped_rifd.hpp
  stream.fwd.hpp
  stream.hpp
  virtual_memory.fwd.hpp
  virtual_memory.hpp
  DESTINATION include/iml/client
)

extended_add_library (NAME client
  NAMESPACE IML
  TYPE SHARED
  SOURCES "iml.cpp"
          "private/option.cpp"
          "private/startup_and_shutdown.cpp"
          "rifd_entry_points.cpp"
          "scoped_rifd.cpp"
          "stream.cpp"
          "virtual_memory.cpp"
  LIBRARIES IML::rif-strategies
            IML::vmem-pc-client-static
            IML::vmem-pc-segment
            IML::vmem-client
            IML::fhg-revision
            Util::Generic
            Boost::filesystem
            Boost::program_options
  INSTALL
)

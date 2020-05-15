# bernd.loerwald@itwm.fraunhofer.de

add_subdirectory (worker)

install (FILES
  certificates.hpp
  client.fwd.hpp
  client.hpp
  drts.fwd.hpp
  drts.hpp
  pimpl.hpp
  rifd_entry_points.fwd.hpp
  rifd_entry_points.hpp
  scoped_rifd.fwd.hpp
  scoped_rifd.hpp
  stream.fwd.hpp
  stream.hpp
  information_to_reattach.fwd.hpp
  information_to_reattach.hpp
  virtual_memory.fwd.hpp
  virtual_memory.hpp
  worker_description.hpp
  DESTINATION include/drts/
)

extended_add_library (NAME gspc
  TYPE SHARED
  SOURCES "client.cpp"
          "drts.cpp"
          "information_to_reattach.cpp"
          "private/option.cpp"
          "private/startup_and_shutdown.cpp"
          "rifd_entry_points.cpp"
          "scoped_rifd.cpp"
          "stream.cpp"
          "virtual_memory.cpp"
  LIBRARIES sdpa
            pnet
            rif-strategies
            gpi-space-pc-client-static
            gpi-space-pc-segment
            GPISpace::vmem
            fhg-revision
            Util::Generic
            Boost::filesystem
            Boost::program_options
  INSTALL
)

add_subdirectory (test/add_worker)
add_subdirectory (test/remove_worker)
add_subdirectory (test/parallel_running_workflows)
add_subdirectory (test/put_token)
add_subdirectory (test/workflow_response)
add_subdirectory (test/forbid_double_worker_instances)
add_subdirectory (test/use_fixed_ports_for_agents_and_workers)
add_subdirectory (test/multiple_module_implementations)
add_subdirectory (test/work_stealing)
add_subdirectory (test)

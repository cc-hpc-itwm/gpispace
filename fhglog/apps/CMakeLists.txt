# bernd.loerwald@itwm.fraunhofer.de

fhg_add_runtime_executable (NAME fhglog-dump
  SOURCES "dump.cpp"
  LIBRARIES fhglog
            Boost::program_options
            Util::Generic
)

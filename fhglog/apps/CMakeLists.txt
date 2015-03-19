# bernd.loerwald@itwm.fraunhofer.de

fhg_add_runtime_executable (dump.cpp NAME fhglog-dump
  LINK_LIBRARIES fhglog Boost::program_options Util::Generic
)

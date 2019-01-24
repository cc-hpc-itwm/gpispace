#! \todo unit tests for stencil cache
add_unit_test (NAME gspc_stencil_cache_2D_standalone
  SOURCES "standalone.cpp"
  ARGS --X 200 --Y 200 --R 3 --M 1000 --C 6 --L 3
  USE_BOOST
  LIBRARIES GSPC::StencilCache
            Util::Generic
            Boost::base
            Boost::program_options
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_SOURCE_DIR}"
)

add_unit_test (NAME gspc_Cache
  SOURCES "Cache.cpp"
  USE_BOOST
  LIBRARIES GSPC::Cache
            Util::Generic
)

add_subdirectory (stencil_cache)

add_unit_test (NAME gspc_Forest
  SOURCES "Forest.cpp"
  USE_BOOST
  LIBRARIES Util::Generic
)

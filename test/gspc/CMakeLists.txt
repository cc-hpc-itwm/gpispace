add_unit_test (NAME gspc_Forest
  SOURCES "Forest.cpp"
  USE_BOOST
  LIBRARIES Util::Generic
)

add_unit_test (NAME gspc_StorageWithEvict
  SOURCES "StorageWithEvict.cpp"
  USE_BOOST
  LIBRARIES Util::Generic
)

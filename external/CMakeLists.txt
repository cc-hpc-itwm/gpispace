extended_add_library (NAMESPACE BeeGFS NAME ClientDevel
  SYSTEM_INCLUDE_DIRECTORIES INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}/beegfs-client-devel"
)

extended_add_library (NAME rapidxml
  SYSTEM_INCLUDE_DIRECTORIES INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/rapidxml"
)

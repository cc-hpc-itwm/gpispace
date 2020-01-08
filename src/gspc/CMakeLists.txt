extended_add_library (NAME Cache
  NAMESPACE GSPC
  LIBRARIES Boost::base
)
extended_add_library (NAME StencilCache
  NAMESPACE GSPC
  LIBRARIES Util::Generic
            Boost::base
)

#! \todo This is actually tied to src/we/type/stencil_cache.cpp
#! rather than this folder and in fact completely unused here. Move
#! there?
install (FILES
  stencil_cache/callback.hpp
  DESTINATION include/gspc/stencil_cache/
)

extended_add_library (NAME gspc19
  SOURCES "ExecutionEnvironment.cpp"
          "ResourceManager.cpp"
          "Resource.cpp"
          "RemoteInterface.cpp"
          "Scheduler.cpp"
          "Workflow.cpp"
  LIBRARIES Util::Generic
            Boost::base
)

extended_add_executable (NAME GSPC19
  SOURCES "GSPC19.cpp"
  LIBRARIES gspc19
            Util::Generic
)

extended_add_executable (NAME Proto
  SOURCES "Proto.cpp"
  LIBRARIES Util::Generic
            gspc::logging
            RPC
            Boost::base
)

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

extended_add_library (NAME util
  NAMESPACE GSPC
  # SOURCES "gspc/util/Forest.ipp"
  LIBRARIES Boost::serialization
  )
extended_add_library (NAME proto
  NAMESPACE GSPC
  SOURCES "Proto.cpp"
          "rpc/TODO.cpp"
          "remote_interface/ID.cpp"
          "resource/ID.cpp"
          "task/ID.cpp"
          "job/ID.cpp"
          "Task.cpp"
          "task/Result.cpp"
          "comm/scheduler/worker/Server.cpp"
          "comm/scheduler/worker/Client.cpp"
          "comm/worker/scheduler/Server.cpp"
          "comm/worker/scheduler/Client.cpp"
  LIBRARIES GSPC::util
            gspc::logging
            Util::Generic
            RPC
            Boost::base
)

extended_add_executable (NAME Proto
  SOURCES "Proto_Main.cpp"
  LIBRARIES GSPC::proto
)

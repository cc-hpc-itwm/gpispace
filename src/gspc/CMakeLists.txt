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

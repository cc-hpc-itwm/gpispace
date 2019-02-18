extended_add_library (NAME Cache
  NAMESPACE GSPC
  LIBRARIES Boost::base
)
extended_add_library (NAME StencilCache
  NAMESPACE GSPC
  LIBRARIES Util::Generic
            Boost::base
)

#! \todo This is actually tied to src/we/type/stencil_caches.cpp
#! rather than this folder and in fact completely unused here. Move
#! there?
install (FILES
  stencil_cache/callback.hpp
  DESTINATION include/gspc/stencil_cache/
)

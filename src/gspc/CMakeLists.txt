extended_add_library (NAME Cache
  NAMESPACE GSPC
  LIBRARIES Boost::base
)
extended_add_library (NAME StencilCache
  NAMESPACE GSPC
  LIBRARIES Util::Generic
            Boost::base
)

install (FILES
  stencil_cache/types.hpp
  stencil_cache/callback.hpp
  DESTINATION include/gspc/stencil_cache/
)

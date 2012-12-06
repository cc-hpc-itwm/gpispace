# -*- mode: cmake; -*-
set (hdrs
  exceptions.hpp
  IModule.hpp
  loader.hpp
  macros.hpp
  module_call.hpp
  Module.hpp
  module_traits.hpp
  types.hpp
  putget.hpp
  api-guard.hpp
  )

set (srcs
  loader.cpp
)

add_library (we-loader STATIC ${srcs})

add_we_library (we-loader)

install(FILES ${hdrs}
  DESTINATION include/we/loader
  COMPONENT headers)

install (TARGETS we-loader
  ARCHIVE
  DESTINATION lib
  COMPONENT static-libraries
  )

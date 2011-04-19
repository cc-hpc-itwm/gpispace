# -*- mode: cmake; -*-
set (hdrs
  exceptions.hpp
  IModule.hpp
  loader.hpp
  macros.hpp
  module.hpp
  Module.hpp
  module_traits.hpp
  types.hpp
  putget.hpp
  )

install(FILES ${hdrs}
  DESTINATION include/we/loader
  COMPONENT headers)

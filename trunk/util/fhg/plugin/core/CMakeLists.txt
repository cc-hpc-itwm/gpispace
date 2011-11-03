set(hdrs
  plugin.hpp
  kernel.hpp
  plugin_kernel_mediator.hpp
  file_storage.hpp
  null_storage.hpp
)

set(srcs
  plugin.cpp
  kernel.cpp
  plugin_kernel_mediator.cpp
  file_storage.cpp
  null_storage.cpp
)

add_library(fhg-plugin-core ${srcs})

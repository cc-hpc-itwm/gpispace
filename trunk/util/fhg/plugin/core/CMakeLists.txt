set(hdrs
  plugin.hpp
  kernel.hpp
  plugin_kernel_mediator.hpp
)

set(srcs
  plugin.cpp
  kernel.cpp
  plugin_kernel_mediator.cpp
)

add_library(fhg-plugin-core ${srcs})

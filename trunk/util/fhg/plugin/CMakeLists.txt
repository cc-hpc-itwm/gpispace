set(hdrs
  kernel.hpp
)

set(srcs
  kernel.cpp
)

add_library(fhg-plugin-core ${srcs})

include(libutils)
merge_static_libs(fhg-plugin fhg-plugin "fhg-plugin-core;${FhgLog_LIBRARY};${Boost_LIBRARIES}")

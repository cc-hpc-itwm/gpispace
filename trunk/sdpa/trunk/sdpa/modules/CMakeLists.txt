#
set(srcs ModuleLoader.cpp Module.cpp )
set(hdrs ModuleLoader.hpp Module.hpp )

ADD_LIBRARY(sdpa-modules ${srcs})
target_link_libraries(sdpa-modules sdpa-wf)

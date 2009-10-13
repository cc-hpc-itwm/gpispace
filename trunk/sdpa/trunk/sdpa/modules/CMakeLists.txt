set(srcs ModuleLoader.cpp Module.cpp FallBackModuleLoader.cpp)
set(hdrs ModuleLoader.hpp Module.hpp FallBackModuleLoader.hpp)

ADD_LIBRARY(sdpa-modules ${srcs})
target_link_libraries(sdpa-modules sdpa-wf)

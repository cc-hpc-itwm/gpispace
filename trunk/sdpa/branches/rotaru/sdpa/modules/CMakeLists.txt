set(srcs ActivityExecutor.cpp                             FallBackModuleLoader.cpp)
set(hdrs ActivityExecutor.hpp ModuleLoader.hpp Module.hpp FallBackModuleLoader.hpp)

ADD_LIBRARY(sdpa-modules ${srcs})
target_link_libraries(sdpa-modules)

install(FILES ${hdrs} DESTINATION include/sdpa/modules)
install(TARGETS sdpa-modules ARCHIVE DESTINATION lib)

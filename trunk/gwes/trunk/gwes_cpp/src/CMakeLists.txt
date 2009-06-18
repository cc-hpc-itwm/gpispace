
file(GLOB hdrs "*.h")
file(GLOB srcs "*.cpp")
ADD_LIBRARY(gwes_cpp ${hdrs} ${srcs})

install(FILES ${hdrs} DESTINATION include/gwes)
install(TARGETS gwes_cpp ARCHIVE DESTINATION lib)

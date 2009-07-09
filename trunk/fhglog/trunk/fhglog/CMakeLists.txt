file(GLOB hdrs "*.hpp")
file(GLOB srcs "*.cpp")

ADD_LIBRARY(fhglog ${hdrs} ${srcs})
#add_dependencies(fhglog foo)

install(FILES ${hdrs} DESTINATION include/fhglog)
install(TARGETS fhglog ARCHIVE DESTINATION lib)

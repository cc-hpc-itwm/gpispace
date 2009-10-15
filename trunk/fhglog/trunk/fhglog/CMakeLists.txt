# -*- mode: cmake; -*-
file(GLOB hdrs "*.hpp")
file(GLOB srcs "*.cpp")

ADD_LIBRARY(fhglog ${hdrs} ${srcs})

install(FILES ${hdrs} DESTINATION include/fhglog)
install(TARGETS fhglog DESTINATION lib
  EXPORT ${PROJECT_NAME}Exports
)

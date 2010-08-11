# -*- mode: cmake; -*-
file(GLOB hdrs "*.hpp")
file(GLOB srcs "*.cpp")

ADD_LIBRARY(fhgcom ${hdrs} ${srcs})
ADD_LIBRARY(fhgcom-shared SHARED ${hdrs} ${srcs})
set_target_properties(fhgcom-shared PROPERTIES OUTPUT_NAME fhgcom)

install(FILES ${hdrs} DESTINATION include/fhgcom)
install(TARGETS fhgcom DESTINATION lib EXPORT ${PROJECT_NAME}Exports)
install(TARGETS fhgcom-shared DESTINATION lib EXPORT ${PROJECT_NAME}Exports)

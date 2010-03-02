# -*- mode: cmake; -*-
file(GLOB hdrs "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")
file(GLOB srcs "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

ADD_LIBRARY(seda STATIC ${hdrs} ${srcs})
#ADD_LIBRARY(sedaShared SHARED ${hdrs} ${srcs})
#set_target_properties(sedaStatic sedaShared PROPERTIES OUTPUT_NAME seda)

if (WITH_LOG4CPP AND LOG4CPP_FOUND)
  target_link_libraries(seda ${LOG4CPP_LIBRARY})
endif (WITH_LOG4CPP AND LOG4CPP_FOUND)

if (WITH_FHGLOG AND FHGLOG_FOUND)
  target_link_libraries(seda ${FHGLOG_LIBRARY})
endif (WITH_FHGLOG AND FHGLOG_FOUND)

target_link_libraries(seda ${Boost_LIBRARIES})

install(FILES ${hdrs} DESTINATION include/seda)
install(TARGETS seda
  DESTINATION lib
#  CONFIGURATIONS Release
  EXPORT ${PROJECT_NAME}Exports)

#install(TARGETS sedaShared
#	LIBRARY DESTINATION lib)
#install(TARGETS sedaStatic
#	ARCHIVE DESTINATION lib)

#install(TARGETS foo DESTINATION lib EXPORT fooExports)

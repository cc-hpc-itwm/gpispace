file(GLOB hdrs "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")
file(GLOB srcs "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

ADD_LIBRARY(seda STATIC ${hdrs} ${srcs})
#ADD_LIBRARY(sedaShared SHARED ${hdrs} ${srcs})
#set_target_properties(sedaStatic sedaShared PROPERTIES OUTPUT_NAME seda)

install(FILES ${hdrs} DESTINATION include/seda)
install(TARGETS seda
	ARCHIVE DESTINATION lib)
#install(TARGETS sedaShared
#	LIBRARY DESTINATION lib)
#install(TARGETS sedaStatic
#	ARCHIVE DESTINATION lib)

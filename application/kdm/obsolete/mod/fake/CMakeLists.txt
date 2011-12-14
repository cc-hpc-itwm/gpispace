add_library(kdm-fake SHARED kdm.cpp)
set_target_properties(kdm-fake PROPERTIES OUTPUT_NAME kdm)
install(TARGETS kdm-fake
  ARCHIVE
  DESTINATION lib/fake
  COMPONENT modules)

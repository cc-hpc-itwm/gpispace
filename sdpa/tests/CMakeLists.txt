# -*- mode: cmake; -*-

add_subdirectory(sdpa)
add_subdirectory(bin)

fhg_add_application_test (re_register_after_master_connection_lost
  SCRIPT re_register_after_master_connection_lost)

fhg_add_application_test (properly_find_master_when_started_after_agent
  SCRIPT properly_find_master_when_started_after_agent)

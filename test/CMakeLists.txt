include (${CMAKE_CURRENT_SOURCE_DIR}/gspc_add_test.cmake)

if (GSPC_WITH_IML)
  add_subdirectory (many_ranges_in_memory_transfer)
endif()
add_subdirectory (parallel_reduce)
add_subdirectory (share)
add_subdirectory (testing)

add_subdirectory (drts/add_worker)
add_subdirectory (drts/remove_worker)
add_subdirectory (drts/parallel_running_workflows)
add_subdirectory (drts/put_token)
add_subdirectory (drts/workflow_response)
add_subdirectory (drts/fail_when_max_num_retries_is_exceeded)
add_subdirectory (drts/forbid_double_worker_instances)
add_subdirectory (drts/use_fixed_ports_for_agents_and_workers)
add_subdirectory (drts/multiple_module_implementations)
add_subdirectory (drts/work_stealing)
add_subdirectory (drts/run_bunch_of_tasks)
if (GSPC_WITH_IML)
  add_subdirectory (drts/scheduler_performance)
  add_subdirectory (drts/using_ranges_and_buffers_in_memory_transfers)
endif()
add_subdirectory (drts)

add_subdirectory (assert)

add_subdirectory (com)

if (GSPC_WITH_IML)
  add_subdirectory (iml)
endif()

add_subdirectory (logging)

add_subdirectory (monitor)

add_subdirectory (rif)

add_subdirectory (scheduler)
add_subdirectory (scheduler/bin)
add_subdirectory (scheduler/scheduler)
add_subdirectory (scheduler/daemon)

add_subdirectory (rpc)

add_subdirectory (util)

add_subdirectory (we/expr/parse)
add_subdirectory (we/expr)
add_subdirectory (we/loader)
add_subdirectory (we/plugin)
add_subdirectory (we)
add_subdirectory (we/type)

add_subdirectory (xml)

add_test (NAME installed_headers_are_selfcontained
  COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/installed_headers_are_selfcontained.sh"
          "${CMAKE_INSTALL_PREFIX}"
          "${CMAKE_CXX_COMPILER}"
          "-DGSPC_WITH_IML=$<BOOL:${GSPC_WITH_IML}>"
)
set_property (TEST installed_headers_are_selfcontained
  APPEND PROPERTY LABELS "requires_installation"
)
set_property (TEST installed_headers_are_selfcontained
  APPEND PROPERTY REQUIRED_FILES ${FILES_REQUIRED_IN_INSTALLATION}
)

find_program (_xmllint_exe NAMES "xmllint")
if (_xmllint_exe)
  add_test (NAME xpnets_are_valid
    COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/xpnets_are_valid.sh"
            "${PROJECT_SOURCE_DIR}"
            "${_xmllint_exe}"
  )
  set_property (TEST xpnets_are_valid
    APPEND PROPERTY RUN_SERIAL 1
  )
endif()

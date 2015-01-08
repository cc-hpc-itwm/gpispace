include(cmake_parse_arguments)
include(car_cdr_macros)

set (FILES_REQUIRED_IN_INSTALLATION
  "${CMAKE_INSTALL_PREFIX}/bin/agent"
  "${CMAKE_INSTALL_PREFIX}/bin/drts-kernel"
  "${CMAKE_INSTALL_PREFIX}/bin/fhglog-dump"
  "${CMAKE_INSTALL_PREFIX}/bin/gpi-space"
  "${CMAKE_INSTALL_PREFIX}/bin/gspcmonc"
  "${CMAKE_INSTALL_PREFIX}/bin/orchestrator"
  "${CMAKE_INSTALL_PREFIX}/bin/pnet2dot"
  "${CMAKE_INSTALL_PREFIX}/bin/pnetc"
  "${CMAKE_INSTALL_PREFIX}/bin/pnete"
  "${CMAKE_INSTALL_PREFIX}/bin/pnetv"
  "${CMAKE_INSTALL_PREFIX}/bin/sdpa-gui"
  "${CMAKE_INSTALL_PREFIX}/bin/start-and-fork"
  "${CMAKE_INSTALL_PREFIX}/bin/sdpac"
  "${CMAKE_INSTALL_PREFIX}/etc/sdpa/sdpa.env"
  "${CMAKE_INSTALL_PREFIX}/external/boost/include/boost/version.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/drts/client.fwd.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/drts/client.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/drts/drts.fwd.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/drts/drts.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/drts/stream.fwd.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/drts/stream.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/drts/information_to_reattach.fwd.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/drts/information_to_reattach.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/drts/virtual_memory.fwd.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/drts/virtual_memory.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/drts/worker/context.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/drts/worker/context_fwd.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/fhg/plugins/gpi.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/fhg/util/dl.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/fhg/util/parse/error.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/fhg/util/parse/position.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/fhg/util/parse/require.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/fhglog/Appender.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/fhglog/Configuration.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/fhglog/LogMacros.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/fhglog/Logger.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/fhglog/event.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/fhglog/level.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/fvm-pc/pc.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/pnete/plugin/plugin_api.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/pnete/plugin/plugin_base.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/process/process.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/seis/determine_size.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/seis/do_load.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/seis/do_write.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/we/exception.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/we/expr/eval/context.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/we/expr/token/type.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/we/field.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/we/loader/IModule.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/we/loader/api-guard.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/we/loader/macros.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/we/signature_of.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/we/type/bitsetofint.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/we/type/bytearray.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/we/type/id.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/we/type/literal/control.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/we/type/signature.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/we/type/value.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/we/type/value/path/append.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/we/type/value/peek.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/we/type/value/poke.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/we/type/value/read.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/we/type/value/show.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/we/type/value/to_value.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/we/type/value/from_value.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/we/type/value/unwrap.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/we/type/value/wrap.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/we/util/wfhd.h"
  "${CMAKE_INSTALL_PREFIX}/lib/libfhg-plugin.so.1"
  "${CMAKE_INSTALL_PREFIX}/lib/libfhg-util.a"
  "${CMAKE_INSTALL_PREFIX}/lib/libfhg-util.so.1"
  "${CMAKE_INSTALL_PREFIX}/lib/libfhglog.a"
  "${CMAKE_INSTALL_PREFIX}/lib/libfhglog.so.2"
  "${CMAKE_INSTALL_PREFIX}/lib/libgspc.so"
  "${CMAKE_INSTALL_PREFIX}/lib/libmmgr.so.1"
  "${CMAKE_INSTALL_PREFIX}/lib/libwe-dev.so.1"
  "${CMAKE_INSTALL_PREFIX}/lib/libwfhd.so.1"
  "${CMAKE_INSTALL_PREFIX}/libexec/fhg/plugins/gpi.so"
  "${CMAKE_INSTALL_PREFIX}/libexec/fhg/plugins/gpi_compat.so"
  "${CMAKE_INSTALL_PREFIX}/libexec/sdpa/libdetermine_size.so"
  "${CMAKE_INSTALL_PREFIX}/libexec/sdpa/libdo_load.so"
  "${CMAKE_INSTALL_PREFIX}/libexec/sdpa/libdo_write.so"
  "${CMAKE_INSTALL_PREFIX}/libexec/sdpa/libprocess.so"
  "${CMAKE_INSTALL_PREFIX}/libexec/sdpa/scripts/start-drts"
  "${CMAKE_INSTALL_PREFIX}/share/sdpa/make/common.mk"
  "${CMAKE_INSTALL_PREFIX}/share/sdpa/xml/lib/4.xml"
  "${CMAKE_INSTALL_PREFIX}/share/sdpa/xml/lib/5.xml"
  "${CMAKE_INSTALL_PREFIX}/share/sdpa/xml/lib/6.xml"
  "${CMAKE_INSTALL_PREFIX}/share/sdpa/xml/lib/cache/done_with_slot.xpnet"
  "${CMAKE_INSTALL_PREFIX}/share/sdpa/xml/lib/cache/fill.xpnet"
  "${CMAKE_INSTALL_PREFIX}/share/sdpa/xml/lib/cache/get_slot_for_id.xpnet"
  "${CMAKE_INSTALL_PREFIX}/share/sdpa/xml/lib/cache/init.xpnet"
  "${CMAKE_INSTALL_PREFIX}/share/sdpa/xml/lib/cache/maybe_start_to_fill.xpnet"
  "${CMAKE_INSTALL_PREFIX}/share/sdpa/xml/lib/cache/start_to_fill.xpnet"
  "${CMAKE_INSTALL_PREFIX}/share/sdpa/xml/lib/cache/type.xpnet"
  "${CMAKE_INSTALL_PREFIX}/share/sdpa/xml/lib/dup.xml"
  "${CMAKE_INSTALL_PREFIX}/share/sdpa/xml/lib/eatN.xml"
  "${CMAKE_INSTALL_PREFIX}/share/sdpa/xml/lib/grid/nth.xpnet"
  "${CMAKE_INSTALL_PREFIX}/share/sdpa/xml/lib/grid/size.xpnet"
  "${CMAKE_INSTALL_PREFIX}/share/sdpa/xml/lib/grid/type.xpnet"
  "${CMAKE_INSTALL_PREFIX}/share/sdpa/xml/lib/make_pair.xml"
  "${CMAKE_INSTALL_PREFIX}/share/sdpa/xml/lib/memory/global/handle.xpnet"
  "${CMAKE_INSTALL_PREFIX}/share/sdpa/xml/lib/memory/global/range.xpnet"
  "${CMAKE_INSTALL_PREFIX}/share/sdpa/xml/lib/point/type.xpnet"
  "${CMAKE_INSTALL_PREFIX}/share/sdpa/xml/lib/replicate.xpnet"
  "${CMAKE_INSTALL_PREFIX}/share/sdpa/xml/lib/sequence.xml"
  "${CMAKE_INSTALL_PREFIX}/share/sdpa/xml/lib/stream/mark_free.xpnet"
  "${CMAKE_INSTALL_PREFIX}/share/sdpa/xml/lib/stream/work_package.xpnet"
  "${CMAKE_INSTALL_PREFIX}/share/sdpa/xml/lib/sequence/interval.xpnet"
  "${CMAKE_INSTALL_PREFIX}/share/sdpa/xml/lib/sequence/ntom.xpnet"
  "${CMAKE_INSTALL_PREFIX}/share/sdpa/xml/lib/sequence_bounded.xml"
  "${CMAKE_INSTALL_PREFIX}/share/sdpa/xml/lib/sequence_control.xml"
  "${CMAKE_INSTALL_PREFIX}/share/sdpa/xml/lib/tagged_sequence.xml"
  "${CMAKE_INSTALL_PREFIX}/share/sdpa/xml/lib/tagged_sequence_bounded.xml"
  "${CMAKE_INSTALL_PREFIX}/share/sdpa/xml/lib/trigger_if.xml"
  "${CMAKE_INSTALL_PREFIX}/share/sdpa/xml/lib/trigger_when.xml"
  "${CMAKE_INSTALL_PREFIX}/share/sdpa/xml/lib/triple.xml"
  "${CMAKE_INSTALL_PREFIX}/share/sdpa/xml/lib/wait.xml"
  "${CMAKE_INSTALL_PREFIX}/share/sdpa/xml/xsd/pnet.rnc"
  "${CMAKE_INSTALL_PREFIX}/share/sdpa/xml/xsd/pnet.xsd"
  "${CMAKE_INSTALL_PREFIX}/share/sdpa/xml/xsd/schemas.xml"
)

set (TEST_VMEM_PORT_COUNTER 10820 CACHE INTERNAL "counter for vmem-port")
set (TEST_VMEM_PORTS_PER_TEST 100)

macro(FHG_ADD_TEST)
  PARSE_ARGUMENTS(TEST
    "LINK_LIBRARIES;DEPENDS;PROJECT;ARGS;DESCRIPTION;COMPILE_FLAGS;INCLUDE_DIRECTORIES"
    "VERBOSE;BOOST_UNIT_TEST;REQUIRES_INSTALLATION;PERFORMANCE_TEST;REQUIRES_VIRTUAL_MEMORY"
    ${ARGN}
    )
  CAR(TEST_SOURCE ${TEST_DEFAULT_ARGS})
  CDR(TEST_ADDITIONAL_SOURCES ${TEST_DEFAULT_ARGS})

    if (TEST_BOOST_UNIT_TEST)
      set (TEST_LINK_LIBRARIES ${TEST_LINK_LIBRARIES} ${Boost_TEST_EXEC_MONITOR_LIBRARY})
      set (TEST_LINK_LIBRARIES ${TEST_LINK_LIBRARIES} ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
    endif()

    set (TEST_PREFIX "")
    if (TEST_PROJECT)
      set(TEST_PREFIX "${TEST_PROJECT}_")
    endif()

    # get the filename without extension
    string(REGEX REPLACE "(.*/)?(.*)\\.c.*" "${TEST_PREFIX}\\2" tc_name ${TEST_SOURCE})

    if (TEST_REQUIRES_VIRTUAL_MEMORY)
      set (TEST_ARGS ${TEST_ARGS} --virtual-memory-port ${TEST_VMEM_PORT_COUNTER})
      math (EXPR TEST_VMEM_PORT_COUNTER_TMP
                 "${TEST_VMEM_PORT_COUNTER} + ${TEST_VMEM_PORTS_PER_TEST}"
      )
      set (TEST_VMEM_PORT_COUNTER ${TEST_VMEM_PORT_COUNTER_TMP}
        CACHE INTERNAL "NOTE: yep, cmake requires this temporary"
      )
    endif()

    if (TEST_VERBOSE)
      message (STATUS "adding test ${tc_name} ${TEST_ARGS} (${TEST_DESCRIPTION})")
    endif()

    add_executable(${tc_name} ${TEST_SOURCE} ${TEST_ADDITIONAL_SOURCES})
    if (TEST_COMPILE_FLAGS)
      set_target_properties(${tc_name} PROPERTIES COMPILE_FLAGS ${TEST_COMPILE_FLAGS})
    endif()
    if (TEST_INCLUDE_DIRECTORIES)
      target_include_directories (${tc_name} ${TEST_INCLUDE_DIRECTORIES})
    endif()
    target_link_libraries(${tc_name} ${TEST_LINK_LIBRARIES})
    add_test (NAME ${tc_name} COMMAND $<TARGET_FILE:${tc_name}> ${TEST_ARGS})

    get_test_property (${tc_name} LABELS tc_labels)
    if (NOT tc_labels)
      set (tc_labels)
    endif()

    if (TEST_REQUIRES_INSTALLATION)
      set_tests_properties (${tc_name}
        PROPERTIES REQUIRED_FILES "${FILES_REQUIRED_IN_INSTALLATION}"
      )
      list(APPEND tc_labels "requires_installation")
    endif()

    if (TEST_PERFORMANCE_TEST)
      list(APPEND tc_labels "performance_test")
    endif()

    set_tests_properties (${tc_name}
      PROPERTIES LABELS "${tc_labels}"
    )

    foreach (d ${TEST_DEPENDS})
      add_dependencies(${tc_name} ${d})
    endforeach()
endmacro()

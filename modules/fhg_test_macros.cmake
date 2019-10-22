set (FILES_REQUIRED_IN_INSTALLATION
  "${CMAKE_INSTALL_PREFIX}/bin/gspc-bootstrap-rifd"
  "${CMAKE_INSTALL_PREFIX}/bin/gspc-monitor"
  "${CMAKE_INSTALL_PREFIX}/bin/gspc-rifd"
  "${CMAKE_INSTALL_PREFIX}/bin/gspc-teardown-rifd"
  "${CMAKE_INSTALL_PREFIX}/bin/gspcmonc"
  "${CMAKE_INSTALL_PREFIX}/bin/pnet2dot"
  "${CMAKE_INSTALL_PREFIX}/bin/pnetc"
  "${CMAKE_INSTALL_PREFIX}/bin/pnetd"
  "${CMAKE_INSTALL_PREFIX}/bin/pnetv"
  "${CMAKE_INSTALL_PREFIX}/external/boost/include/boost/version.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/drts/client.fwd.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/drts/client.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/drts/drts.fwd.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/drts/drts.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/drts/information_to_reattach.fwd.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/drts/information_to_reattach.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/drts/pimpl.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/drts/rifd_entry_points.fwd.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/drts/rifd_entry_points.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/drts/scoped_rifd.fwd.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/drts/scoped_rifd.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/drts/stream.fwd.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/drts/stream.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/drts/virtual_memory.fwd.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/drts/virtual_memory.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/drts/worker/context.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/drts/worker/context_fwd.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/fhg/util/dl.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/fhg/util/parse/error.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/fhg/util/parse/position.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/fhg/util/parse/require.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/gspc/stencil_cache/callback.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/logging/endpoint.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/logging/endpoint.ipp"
  "${CMAKE_INSTALL_PREFIX}/include/logging/socket_endpoint.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/logging/socket_endpoint.ipp"
  "${CMAKE_INSTALL_PREFIX}/include/logging/tcp_endpoint.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/logging/tcp_endpoint.ipp"
  "${CMAKE_INSTALL_PREFIX}/include/process/process.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/rif/started_process_promise.hpp"
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
  "${CMAKE_INSTALL_PREFIX}/include/we/type/literal/control.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/we/type/signature.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/we/type/value.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/we/type/value/from_value.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/we/type/value/path/append.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/we/type/value/path/join.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/we/type/value/peek.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/we/type/value/peek_or_die.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/we/type/value/poke.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/we/type/value/read.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/we/type/value/serialize.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/we/type/value/show.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/we/type/value/to_value.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/we/type/value/unwrap.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/we/type/value/wrap.hpp"
  "${CMAKE_INSTALL_PREFIX}/lib/libdrts-context.so"
  "${CMAKE_INSTALL_PREFIX}/lib/libfhg-util.a"
  "${CMAKE_INSTALL_PREFIX}/lib/librif-started_process_promise.a"
  "${CMAKE_INSTALL_PREFIX}/lib/libgspc.so"
  "${CMAKE_INSTALL_PREFIX}/lib/libwe-dev.so"
  "${CMAKE_INSTALL_PREFIX}/libexec/gspc/agent"
  "${CMAKE_INSTALL_PREFIX}/libexec/gspc/drts-kernel"
  "${CMAKE_INSTALL_PREFIX}/libexec/gspc/gpi-space"
  "${CMAKE_INSTALL_PREFIX}/libexec/gspc/orchestrator"
  #! \note This does not include bundled libraries!
  "${CMAKE_INSTALL_PREFIX}/libexec/gspc/libdetermine_size.so"
  "${CMAKE_INSTALL_PREFIX}/libexec/gspc/libdo_load.so"
  "${CMAKE_INSTALL_PREFIX}/libexec/gspc/libdo_write.so"
  "${CMAKE_INSTALL_PREFIX}/libexec/gspc/libprocess.a"
  "${CMAKE_INSTALL_PREFIX}/revision"
  "${CMAKE_INSTALL_PREFIX}/git.submodules"
  "${CMAKE_INSTALL_PREFIX}/share/man/man5/xpnet.5"
  "${CMAKE_INSTALL_PREFIX}/share/gspc/xml/lib/4.xml"
  "${CMAKE_INSTALL_PREFIX}/share/gspc/xml/lib/5.xml"
  "${CMAKE_INSTALL_PREFIX}/share/gspc/xml/lib/6.xml"
  "${CMAKE_INSTALL_PREFIX}/share/gspc/xml/lib/cache/done_with_slot.xpnet"
  "${CMAKE_INSTALL_PREFIX}/share/gspc/xml/lib/cache/fill.xpnet"
  "${CMAKE_INSTALL_PREFIX}/share/gspc/xml/lib/cache/get_slot_for_id.xpnet"
  "${CMAKE_INSTALL_PREFIX}/share/gspc/xml/lib/cache/init.xpnet"
  "${CMAKE_INSTALL_PREFIX}/share/gspc/xml/lib/cache/maybe_start_to_fill.xpnet"
  "${CMAKE_INSTALL_PREFIX}/share/gspc/xml/lib/cache/start_to_fill.xpnet"
  "${CMAKE_INSTALL_PREFIX}/share/gspc/xml/lib/cache/type.xpnet"
  "${CMAKE_INSTALL_PREFIX}/share/gspc/xml/lib/dup.xml"
  "${CMAKE_INSTALL_PREFIX}/share/gspc/xml/lib/eatN.xml"
  "${CMAKE_INSTALL_PREFIX}/share/gspc/xml/lib/grid/nth.xpnet"
  "${CMAKE_INSTALL_PREFIX}/share/gspc/xml/lib/grid/size.xpnet"
  "${CMAKE_INSTALL_PREFIX}/share/gspc/xml/lib/grid/type.xpnet"
  "${CMAKE_INSTALL_PREFIX}/share/gspc/xml/lib/make_pair.xml"
  "${CMAKE_INSTALL_PREFIX}/share/gspc/xml/lib/memory/global/handle.xpnet"
  "${CMAKE_INSTALL_PREFIX}/share/gspc/xml/lib/memory/global/range.xpnet"
  "${CMAKE_INSTALL_PREFIX}/share/gspc/xml/lib/point/type.xpnet"
  "${CMAKE_INSTALL_PREFIX}/share/gspc/xml/lib/replicate.xpnet"
  "${CMAKE_INSTALL_PREFIX}/share/gspc/xml/lib/sequence.xml"
  "${CMAKE_INSTALL_PREFIX}/share/gspc/xml/lib/sequence/interval.xpnet"
  "${CMAKE_INSTALL_PREFIX}/share/gspc/xml/lib/sequence/ntom.xpnet"
  "${CMAKE_INSTALL_PREFIX}/share/gspc/xml/lib/sequence_bounded.xml"
  "${CMAKE_INSTALL_PREFIX}/share/gspc/xml/lib/sequence_control.xml"
  "${CMAKE_INSTALL_PREFIX}/share/gspc/xml/lib/stream/mark_free.xpnet"
  "${CMAKE_INSTALL_PREFIX}/share/gspc/xml/lib/stream/work_package.xpnet"
  "${CMAKE_INSTALL_PREFIX}/share/gspc/xml/lib/tagged_sequence.xml"
  "${CMAKE_INSTALL_PREFIX}/share/gspc/xml/lib/tagged_sequence_bounded.xml"
  "${CMAKE_INSTALL_PREFIX}/share/gspc/xml/lib/trigger_if.xml"
  "${CMAKE_INSTALL_PREFIX}/share/gspc/xml/lib/trigger_when.xml"
  "${CMAKE_INSTALL_PREFIX}/share/gspc/xml/lib/triple.xml"
  "${CMAKE_INSTALL_PREFIX}/share/gspc/xml/lib/wait.xml"
  "${CMAKE_INSTALL_PREFIX}/share/gspc/xml/xsd/pnet.rnc"
  "${CMAKE_INSTALL_PREFIX}/share/gspc/xml/xsd/pnet.xsd"
  "${CMAKE_INSTALL_PREFIX}/share/gspc/xml/xsd/schemas.xml"
)

set (TEST_VMEM_PORT_COUNTER 10820 CACHE INTERNAL "counter for vmem-port")
set (TEST_VMEM_PORTS_PER_TEST 100)

#! \note due to half of the arguments not being known and them thus
#! not being split (eg LIBRARIES is part of ARGS), appending would
#! mean we might append into some other argument's parameters
macro (prepend _list)
  list (INSERT ${_list} 0 ${ARGN})
endmacro()

macro(FHG_ADD_TEST)
  set (options REQUIRES_INSTALLATION REQUIRES_VIRTUAL_MEMORY START_SCOPED_RIF)
  set (one_value_options)
  set (multi_value_options LABELS ARGS)
  set (required_options)
  _parse_arguments_with_unknown (TEST "${options}" "${one_value_options}" "${multi_value_options}" "${required_options}" ${ARGN})

  set (_fwd_args)

  if (TEST_REQUIRES_INSTALLATION)
    list (APPEND _fwd_args REQUIRED_FILES ${FILES_REQUIRED_IN_INSTALLATION})
    prepend (TEST_LABELS "requires_installation")
  endif()

  if (TEST_REQUIRES_VIRTUAL_MEMORY)
    prepend (TEST_ARGS --virtual-memory-port ${TEST_VMEM_PORT_COUNTER})
    math (EXPR TEST_VMEM_PORT_COUNTER_TMP
               "${TEST_VMEM_PORT_COUNTER} + ${TEST_VMEM_PORTS_PER_TEST}"
    )
    set (TEST_VMEM_PORT_COUNTER ${TEST_VMEM_PORT_COUNTER_TMP}
      CACHE INTERNAL "NOTE: yep, cmake requires this temporary"
    )
    prepend (TEST_LABELS "requires_vmem")
    list (APPEND _fwd_args RUN_SERIAL)
  endif()

  if (TEST_START_SCOPED_RIF)
    prepend (TEST_ARGS --rif-strategy ${TESTING_RIF_STRATEGY})
    if (TESTING_RIF_STRATEGY_PARAMETERS)
      prepend (TEST_ARGS RIF ${TESTING_RIF_STRATEGY_PARAMETERS} FIR)
    endif()
  endif()

  add_unit_test (${TEST_UNPARSED_ARGUMENTS}
    LABELS ${TEST_LABELS}
    ARGS ${TEST_ARGS}
    ${_fwd_args}
  )
endmacro()

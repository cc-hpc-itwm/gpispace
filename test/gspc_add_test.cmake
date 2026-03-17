# Copyright (C) 2011-2016,2018-2024,2026 Fraunhofer ITWM
# SPDX-License-Identifier: GPL-3.0-or-later

set (_installed_lib_dir
  "${CMAKE_INSTALL_PREFIX}/${GSPC_LIBRARY_INSTALL_DIR}"
)

set (FILES_REQUIRED_IN_INSTALLATION
  "${CMAKE_INSTALL_PREFIX}/bin/gspc-bootstrap-rifd"
  $<$<BOOL:${GSPC_WITH_MONITOR_APP}>:${CMAKE_INSTALL_PREFIX}/bin/gspc-monitor>
  "${CMAKE_INSTALL_PREFIX}/bin/gspc-rifd"
  "${CMAKE_INSTALL_PREFIX}/bin/gspc-teardown-rifd"
  "${CMAKE_INSTALL_PREFIX}/bin/pnet2dot"
  "${CMAKE_INSTALL_PREFIX}/bin/pnetc"
  "${CMAKE_INSTALL_PREFIX}/external/boost/include/boost/version.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/gspc/drts/client.fwd.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/gspc/drts/client.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/gspc/drts/drts.fwd.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/gspc/drts/drts.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/gspc/drts/information_to_reattach.fwd.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/gspc/drts/information_to_reattach.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/gspc/drts/pimpl.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/gspc/drts/rifd_entry_points.fwd.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/gspc/drts/rifd_entry_points.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/gspc/drts/scoped_rifd.fwd.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/gspc/drts/scoped_rifd.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/gspc/drts/stream.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/gspc/drts/virtual_memory.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/gspc/drts/worker/context.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/gspc/drts/worker/context_fwd.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/gspc/detail/export.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/gspc/detail/export.ipp"
  "${CMAKE_INSTALL_PREFIX}/include/gspc/logging/endpoint.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/gspc/logging/endpoint.ipp"
  "${CMAKE_INSTALL_PREFIX}/include/gspc/logging/socket_endpoint.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/gspc/logging/socket_endpoint.ipp"
  "${CMAKE_INSTALL_PREFIX}/include/gspc/logging/tcp_endpoint.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/gspc/logging/tcp_endpoint.ipp"
  "${CMAKE_INSTALL_PREFIX}/include/gspc/rif/started_process_promise.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/gspc/we/exception.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/gspc/we/expr/eval/context.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/gspc/we/expr/token/type.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/gspc/we/field.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/gspc/we/loader/IModule.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/gspc/we/loader/api-guard.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/gspc/we/loader/macros.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/gspc/we/signature_of.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/gspc/we/type/bitsetofint.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/gspc/we/type/bytearray.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/gspc/we/type/literal/control.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/gspc/we/type/signature.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/gspc/we/type/value.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/gspc/we/type/value/from_value.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/gspc/we/type/value/path/append.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/gspc/we/type/value/path/join.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/gspc/we/type/value/peek.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/gspc/we/type/value/peek_or_die.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/gspc/we/type/value/poke.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/gspc/we/type/value/read.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/gspc/we/type/value/serialize.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/gspc/we/type/value/show.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/gspc/we/type/value/to_value.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/gspc/we/type/value/unwrap.hpp"
  "${CMAKE_INSTALL_PREFIX}/include/gspc/we/type/value/wrap.hpp"
  "${_installed_lib_dir}/libgspc_api_guard.so"
  "${_installed_lib_dir}/libgspc_worker_context.so"
  "${_installed_lib_dir}/libgspc.so"
  "${_installed_lib_dir}/libgspc_we.so"
  "${CMAKE_INSTALL_PREFIX}/libexec/gspc/gspc-agent"
  "${CMAKE_INSTALL_PREFIX}/libexec/gspc/gspc-drts-kernel"
  #! \note This does not include bundled libraries!
  "${CMAKE_INSTALL_PREFIX}/gspc_version"
  "${CMAKE_INSTALL_PREFIX}/${GSPC_SHARE_INSTALL_DIR}/README.md"
  "${CMAKE_INSTALL_PREFIX}/${GSPC_SHARE_INSTALL_DIR}/man/man5/xpnet.5"
  "${CMAKE_INSTALL_PREFIX}/${GSPC_SHARE_INSTALL_DIR}/xml/lib/4.xml"
  "${CMAKE_INSTALL_PREFIX}/${GSPC_SHARE_INSTALL_DIR}/xml/lib/5.xml"
  "${CMAKE_INSTALL_PREFIX}/${GSPC_SHARE_INSTALL_DIR}/xml/lib/6.xml"
  "${CMAKE_INSTALL_PREFIX}/${GSPC_SHARE_INSTALL_DIR}/xml/lib/cache/done_with_slot.xpnet"
  "${CMAKE_INSTALL_PREFIX}/${GSPC_SHARE_INSTALL_DIR}/xml/lib/cache/fill.xpnet"
  "${CMAKE_INSTALL_PREFIX}/${GSPC_SHARE_INSTALL_DIR}/xml/lib/cache/get_slot_for_id.xpnet"
  "${CMAKE_INSTALL_PREFIX}/${GSPC_SHARE_INSTALL_DIR}/xml/lib/cache/init.xpnet"
  "${CMAKE_INSTALL_PREFIX}/${GSPC_SHARE_INSTALL_DIR}/xml/lib/cache/maybe_start_to_fill.xpnet"
  "${CMAKE_INSTALL_PREFIX}/${GSPC_SHARE_INSTALL_DIR}/xml/lib/cache/start_to_fill.xpnet"
  "${CMAKE_INSTALL_PREFIX}/${GSPC_SHARE_INSTALL_DIR}/xml/lib/cache/type.xpnet"
  "${CMAKE_INSTALL_PREFIX}/${GSPC_SHARE_INSTALL_DIR}/xml/lib/dup.xml"
  "${CMAKE_INSTALL_PREFIX}/${GSPC_SHARE_INSTALL_DIR}/xml/lib/eatN.xml"
  "${CMAKE_INSTALL_PREFIX}/${GSPC_SHARE_INSTALL_DIR}/xml/lib/grid/nth.xpnet"
  "${CMAKE_INSTALL_PREFIX}/${GSPC_SHARE_INSTALL_DIR}/xml/lib/grid/size.xpnet"
  "${CMAKE_INSTALL_PREFIX}/${GSPC_SHARE_INSTALL_DIR}/xml/lib/grid/type.xpnet"
  "${CMAKE_INSTALL_PREFIX}/${GSPC_SHARE_INSTALL_DIR}/xml/lib/make_pair.xml"
  "${CMAKE_INSTALL_PREFIX}/${GSPC_SHARE_INSTALL_DIR}/xml/lib/memory/global/handle.xpnet"
  "${CMAKE_INSTALL_PREFIX}/${GSPC_SHARE_INSTALL_DIR}/xml/lib/memory/global/range.xpnet"
  "${CMAKE_INSTALL_PREFIX}/${GSPC_SHARE_INSTALL_DIR}/xml/lib/point/type.xpnet"
  "${CMAKE_INSTALL_PREFIX}/${GSPC_SHARE_INSTALL_DIR}/xml/lib/replicate.xpnet"
  "${CMAKE_INSTALL_PREFIX}/${GSPC_SHARE_INSTALL_DIR}/xml/lib/sequence.xml"
  "${CMAKE_INSTALL_PREFIX}/${GSPC_SHARE_INSTALL_DIR}/xml/lib/sequence/interval.xpnet"
  "${CMAKE_INSTALL_PREFIX}/${GSPC_SHARE_INSTALL_DIR}/xml/lib/sequence/ntom.xpnet"
  "${CMAKE_INSTALL_PREFIX}/${GSPC_SHARE_INSTALL_DIR}/xml/lib/sequence_bounded.xml"
  "${CMAKE_INSTALL_PREFIX}/${GSPC_SHARE_INSTALL_DIR}/xml/lib/sequence_control.xml"
  "${CMAKE_INSTALL_PREFIX}/${GSPC_SHARE_INSTALL_DIR}/xml/lib/stream/mark_free.xpnet"
  "${CMAKE_INSTALL_PREFIX}/${GSPC_SHARE_INSTALL_DIR}/xml/lib/stream/work_package.xpnet"
  "${CMAKE_INSTALL_PREFIX}/${GSPC_SHARE_INSTALL_DIR}/xml/lib/tagged_sequence.xml"
  "${CMAKE_INSTALL_PREFIX}/${GSPC_SHARE_INSTALL_DIR}/xml/lib/tagged_sequence_bounded.xml"
  "${CMAKE_INSTALL_PREFIX}/${GSPC_SHARE_INSTALL_DIR}/xml/lib/trigger_if.xml"
  "${CMAKE_INSTALL_PREFIX}/${GSPC_SHARE_INSTALL_DIR}/xml/lib/trigger_when.xml"
  "${CMAKE_INSTALL_PREFIX}/${GSPC_SHARE_INSTALL_DIR}/xml/lib/triple.xml"
  "${CMAKE_INSTALL_PREFIX}/${GSPC_SHARE_INSTALL_DIR}/xml/lib/wait.xml"
  "${CMAKE_INSTALL_PREFIX}/${GSPC_SHARE_INSTALL_DIR}/xml/xsd/pnet.rnc"
  "${CMAKE_INSTALL_PREFIX}/${GSPC_SHARE_INSTALL_DIR}/xml/xsd/pnet.xsd"
  "${CMAKE_INSTALL_PREFIX}/${GSPC_SHARE_INSTALL_DIR}/xml/xsd/schemas.xml"
)

# \todo Assert IML API here? These are duplicated from IML's
# iml_files_in_installation variable, but that's defined after this
# file is included. It could be contained in a `find_package (IML)`,
# but that currently doesn't exist. Alternatively one could probably
# abuse some fake target and property and a generator expression?
if (GSPC_WITH_IML)
  list (APPEND FILES_REQUIRED_IN_INSTALLATION
    "${CMAKE_INSTALL_PREFIX}/include/gspc/drts/drts_iml.hpp"
    "${CMAKE_INSTALL_PREFIX}/include/gspc/drts/stream.fwd.hpp"
    "${CMAKE_INSTALL_PREFIX}/include/gspc/drts/virtual_memory.fwd.hpp"
    "${CMAKE_INSTALL_PREFIX}/include/gspc/iml/AllocationHandle.hpp"
    "${CMAKE_INSTALL_PREFIX}/include/gspc/iml/AllocationHandle.ipp"
    "${CMAKE_INSTALL_PREFIX}/include/gspc/iml/Client.hpp"
    "${CMAKE_INSTALL_PREFIX}/include/gspc/iml/MemcpyID.hpp"
    "${CMAKE_INSTALL_PREFIX}/include/gspc/iml/MemoryLocation.hpp"
    "${CMAKE_INSTALL_PREFIX}/include/gspc/iml/MemoryLocation.ipp"
    "${CMAKE_INSTALL_PREFIX}/include/gspc/iml/MemoryOffset.hpp"
    "${CMAKE_INSTALL_PREFIX}/include/gspc/iml/MemoryRegion.hpp"
    "${CMAKE_INSTALL_PREFIX}/include/gspc/iml/MemoryRegion.ipp"
    "${CMAKE_INSTALL_PREFIX}/include/gspc/iml/MemorySize.hpp"
    "${CMAKE_INSTALL_PREFIX}/include/gspc/iml/Rifs.hpp"
    "${CMAKE_INSTALL_PREFIX}/include/gspc/iml/RuntimeSystem.hpp"
    "${CMAKE_INSTALL_PREFIX}/include/gspc/iml/SegmentAndAllocation.hpp"
    "${CMAKE_INSTALL_PREFIX}/include/gspc/iml/SegmentDescription.hpp"
    "${CMAKE_INSTALL_PREFIX}/include/gspc/iml/SegmentHandle.hpp"
    "${CMAKE_INSTALL_PREFIX}/include/gspc/iml/SegmentHandle.ipp"
    "${CMAKE_INSTALL_PREFIX}/include/gspc/iml/SharedMemoryAllocation.hpp"
    "${CMAKE_INSTALL_PREFIX}/include/gspc/iml/SharedMemoryAllocationHandle.hpp"
    "${CMAKE_INSTALL_PREFIX}/include/gspc/iml/SharedMemoryAllocationHandle.ipp"
    "${CMAKE_INSTALL_PREFIX}/include/gspc/iml/beegfs/SegmentDescription.hpp"
    "${CMAKE_INSTALL_PREFIX}/include/gspc/iml/beegfs/SegmentDescription.ipp"
    "${CMAKE_INSTALL_PREFIX}/include/gspc/iml/gaspi/NetdevID.hpp"
    "${CMAKE_INSTALL_PREFIX}/include/gspc/iml/gaspi/NetdevID.ipp"
    "${CMAKE_INSTALL_PREFIX}/include/gspc/iml/gaspi/SegmentDescription.hpp"
    "${CMAKE_INSTALL_PREFIX}/include/gspc/iml/gaspi/SegmentDescription.ipp"
    "${CMAKE_INSTALL_PREFIX}/include/gspc/iml/rif/EntryPoints.hpp"
    "${_installed_lib_dir}/libgspc_iml_client.so"
    "${CMAKE_INSTALL_PREFIX}/libexec/iml/gspc-iml-gpi-server"
    # Intentionally omitted: libgspc_iml_installation.so:
    # location depends on implementation detail.
  )
endif()

set (TEST_VMEM_PORT_COUNTER 10820 CACHE INTERNAL "counter for vmem-port")
set (TEST_VMEM_PORTS_PER_TEST 100)

#! \note due to half of the arguments not being known and them thus
#! not being split (eg LIBRARIES is part of ARGS), appending would
#! mean we might append into some other argument's parameters
macro (prepend _list)
  list (INSERT ${_list} 0 ${ARGN})
endmacro()

function (gspc_add_test)
  set (options REQUIRES_INSTALLATION REQUIRES_VIRTUAL_MEMORY START_SCOPED_RIF)
  set (one_value_options NAME)
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
    prepend (TEST_ARGS --rif-strategy ${GSPC_TESTING_RIF_STRATEGY})
    if (GSPC_TESTING_RIF_STRATEGY_PARAMETERS)
      prepend (TEST_ARGS RIF ${GSPC_TESTING_RIF_STRATEGY_PARAMETERS} FIR)
    endif()
  endif()

  add_unit_test (NAME ${TEST_NAME}
    ${TEST_UNPARSED_ARGUMENTS}
    LABELS ${TEST_LABELS}
    ARGS ${TEST_ARGS}
    ${_fwd_args}
  )

  if (TEST_REQUIRES_INSTALLATION)
    # When unit tests use our installation, they still see the
    # uninstalled installation sentinels. As our build directory
    # structure differs from the install directory binaries would not
    # be found. Thus, override with the actual installation directory.
    set_tests_properties (${TEST_NAME} PROPERTIES ENVIRONMENT
      "IML_TESTING_OVERRIDE_INSTALLATION_PREFIX=${CMAKE_INSTALL_PREFIX}"
    )
  endif()
endfunction()

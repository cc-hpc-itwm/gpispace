list (TRANSFORM
  TESTING_RIF_STRATEGY_PARAMETERS
  PREPEND
    "--rif-strategy-parameters="
  OUTPUT_VARIABLE
    _testing_rif_strategy_parameters_with_arg_prefix
)

add_test (
  NAME
    doc_example_stochastic_with_heureka
  COMMAND
    selftest
    ${CMAKE_COMMAND}
    "${TESTING_RIF_STRATEGY}"
    ${_testing_rif_strategy_parameters_with_arg_prefix}
  WORKING_DIRECTORY
    ${CMAKE_INSTALL_PREFIX}/${GSPC_SHARE_INSTALL_DIR}/doc/example/stochastic_with_heureka
)

set_property (
  TEST
    doc_example_stochastic_with_heureka
  PROPERTY ENVIRONMENT
    "CXX=${CMAKE_CXX_COMPILER}"
    "GPISpace_ROOT=${CMAKE_INSTALL_PREFIX}"
    "SWH_INSTALL_DIR=${SHARED_DIRECTORY_FOR_TESTS}/stochastic_with_heureka"
)

set_property (
  TEST
    doc_example_stochastic_with_heureka
  PROPERTY LABELS
    "requires_installation"
)

set_property (
  TEST
    doc_example_stochastic_with_heureka
  PROPERTY REQUIRED_FILES
    "${FILES_REQUIRED_IN_INSTALLATION}"
)

###############################################################################
# Includes
###############################################################################

include (util-cmake/add_system_test)

###############################################################################
# Definitions
###############################################################################

# the notation $ENV\{<var-name>} is necessary to delay the variable
# evaluation until the system test is run
set (_gspc_testing_arguments
  "--gspc-home=${CMAKE_INSTALL_PREFIX}"
  "--nodefile=$ENV\{GSPC_NODEFILE_FOR_TESTS}"
  "--rif-strategy=${TESTING_RIF_STRATEGY}"
)

###############################################################################
# System Tests
###############################################################################

util_cmake_add_system_test (aggregate_sum
  CMAKE_DEFINITIONS
    -D GPISpace_ROOT=${CMAKE_INSTALL_PREFIX}
  ARGS
    ${_gspc_testing_arguments}
    --topology=worker:2
    --N=100
)

util_cmake_add_system_test (preferences_and_multimodules
  CMAKE_DEFINITIONS
    -D GPISpace_ROOT=${CMAKE_INSTALL_PREFIX}
  ARGS
    ${_gspc_testing_arguments}
    --num-tasks=5
    --num-workers-per-target 5 4 3
)

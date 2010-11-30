cmake_minimum_required(VERSION 2.6.2)

project(KDM_SDPA)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/modules" "$ENV{CMAKE_MODULE_PATH}")
message(STATUS "MODULE_PATH := ${CMAKE_MODULE_PATH}")

option(ENABLE_OBSOLETE_KDM "Build obsolete generator binaries" No)

if (ENABLE_OBSOLETE_KDM)
  add_subdirectory(obsolete)
endif (ENABLE_OBSOLETE_KDM)

add_subdirectory(src/fake)

include_directories(./src
		    ./src/include
		    ./src/structures
		    ./src/filehandler
		    ./src/utils
                    )

set(SRC ./src/main.cpp
	./src/MigVol.cpp
	./src/MigSubVol.cpp
	./src/TravTimeTab.cpp
        ./src/TraceData.cpp
	./src/TraceBunch.cpp
	./src/tracemem.cpp
        ./src/ttvmmemhandler.cpp
	./src/sdpa_migrate.cpp
        ./src/sdpa_init.cpp
        ./src/sdpa_loadalltraces.cpp
        ./src/sdpa_writeoutvol2disk.cpp
        ./src/sdpa_initsubvol.cpp
	./src/sdpa_loadallTT.cpp
	./src/structures/grid2d.cpp
	./src/structures/grid3d.cpp
	./src/structures/deg.cpp
	./src/structures/migrationjob.cpp
	./src/structures/mod.cpp
	./src/structures/tracingjob.cpp
	./src/structures/spherical.cpp
	./src/structures/triangle.cpp
	./src/structures/coordinates.cpp
	./src/filehandler/checkreadmigrationjob.cpp
	./src/filehandler/migrationfilehandler.cpp
	./src/filehandler/tracefilehandler.cpp
	./src/filehandler/tracememhandler.cpp
	./src/filehandler/ttfilehandler.cpp
	./src/filehandler/checkreadtracingjob.cpp
	./src/filehandler/seismicfilewriter.cpp
        ./src/utils/loggingclass.cpp
	./src/utils/swap_bytes.cpp
	./src/utils/sincinterpolator.cpp
	./src/utils/fft.cpp
	./src/utils/taper.cpp
	./src/utils/filename.cpp
	./src/utils/Acq_geometry.cpp
	./src/utils/XMLReader_red.cpp
	./src/utils/MarkupSTL_red.cpp
        ./src/utils/VM_ParallelEnvironment.cpp
	./src/SeismicRayTracer/receivergrid.cpp
	./src/SeismicRayTracer/receiver.cpp
)

# sdpa kdm module
add_definitions(-DSHMEM -DDEBUGALLOC -DDEBUGCOMM  -DSHMEM)

## check for boost
set(Boost_ADDITIONAL_VERSIONS "1.36" "1.36.0" "1.37" "1.37.0" "1.40" "1.40.0")
set(Boost_FIND_QUIETLY false)
find_package(Boost 1.36 REQUIRED COMPONENTS thread system filesystem serialization program_options)
# we need at least 1.36 so check for it
if (Boost_MAJOR_VERSION LESS 1)
    message(FATAL_ERROR "At least Boost 1.36 is required")
endif(Boost_MAJOR_VERSION LESS 1)
if (Boost_MINOR_VERSION LESS 36)
    message(FATAL_ERROR "At least Boost 1.36 is required")
endif (Boost_MINOR_VERSION LESS 36)
message(STATUS "Boost: -I${Boost_INCLUDE_DIRS} -L${Boost_LIBRARY_DIRS} -l${Boost_LIBRARIES}")
include_directories(${Boost_INCLUDE_DIRS})

find_package(FVM REQUIRED)
include_directories(${FVM_INCLUDE_DIR})

find_package(WE REQUIRED)
include_directories(${WE_INCLUDE_DIR})

find_package(UTIL REQUIRED)
include_directories(${UTIL_INCLUDE_DIR})

find_package(FhgLog REQUIRED)
include_directories (${FhgLog_INCLUDE_DIR})

set(KDM_MOD_INITIALIZE_SRCS
	./src/structures/migrationjob.cpp
   	./src/filehandler/checkreadmigrationjob.cpp
   	./src/utils/XMLReader_red.cpp
	./src/utils/MarkupSTL_red.cpp
	./src/filehandler/tracefilehandler.cpp
	./src/utils/swap_bytes.cpp
	./src/TraceBunch.cpp
        ./src/TraceData.cpp
	./src/utils/fft.cpp
	./src/structures/mod.cpp
        ./src/ttvmmemhandler.cpp
	./src/structures/grid2d.cpp
	./src/structures/grid3d.cpp
	./src/filehandler/migrationfilehandler.cpp
	./src/filehandler/seismicfilewriter.cpp
)

set(KDM_MOD_INITIALIZE_SUBVOL_SRCS
	./src/MigVol.cpp
	./src/MigSubVol.cpp
)

set(KDM_MOD_PROCESS_SRCS
	./src/sdpa_migrate.cpp
	./src/TravTimeTab.cpp
)

set(KDM_MOD_SRCS src/kdm_mod.cpp
  ${KDM_MOD_INITIALIZE_SRCS}
  ${KDM_MOD_INITIALIZE_SUBVOL_SRCS}
  ${KDM_MOD_PROCESS_SRCS}
)

set (KDM_COMPLEX_MOD_SRCS src/kdm_complex_mod.cpp
  ${KDM_MOD_INITIALIZE_SRCS}
  ${KDM_MOD_INITIALIZE_SUBVOL_SRCS}
  ${KDM_MOD_PROCESS_SRCS}
)

set (KDM_FULL_MOD_SRCS src/kdmfull.cpp
  ${KDM_MOD_INITIALIZE_SRCS}
  ${KDM_MOD_INITIALIZE_SUBVOL_SRCS}
  ${KDM_MOD_PROCESS_SRCS}
)

set (KDM_FULL2_MOD_SRCS src/kdmfull2.cpp
  ${KDM_MOD_INITIALIZE_SRCS}
  ${KDM_MOD_INITIALIZE_SUBVOL_SRCS}
  ${KDM_MOD_PROCESS_SRCS}
)

set (KDM_FULL_GENCONF_MOD_SRCS src/genconfig.cpp
  ${KDM_MOD_INITIALIZE_SRCS}
  ${KDM_MOD_INITIALIZE_SUBVOL_SRCS}
  ${KDM_MOD_PROCESS_SRCS}
)

set (STANDALONE_SRCS src/standalone.cpp
    	./src/structures/migrationjob.cpp
	./src/TraceBunch.cpp
        ./src/TraceData.cpp
	./src/filehandler/tracefilehandler.cpp
	./src/structures/mod.cpp
	./src/utils/swap_bytes.cpp
	./src/utils/fft.cpp
	./src/utils/sincinterpolator.cpp
 )

set (FILTER_MOD_SRCS src/filter_mod.cpp
     ${STANDALONE_SRCS}
    )

set (SIMPLE_PROCESS_MOD_SRCS src/simple_process.cpp
     ${STANDALONE_SRCS}
    )

set (FILTER_TRACE_MOD_SRCS src/filter_trace_mod.cpp
     ${STANDALONE_SRCS}
    )

set(SINC_INT_MOD_SRCS
	./src/sinc_mod.cpp
	./src/utils/sincinterpolator.cpp
	./src/utils/taper.cpp
	./src/filehandler/ttfilehandler.cpp
	./src/structures/grid3d.cpp
	./src/SeismicRayTracer/receivergrid.cpp
	./src/utils/swap_bytes.cpp
	./src/SeismicRayTracer/receiver.cpp
	./src/structures/spherical.cpp
	./src/filehandler/checkreadtracingjob.cpp
	./src/utils/XMLReader_red.cpp
	./src/utils/MarkupSTL_red.cpp
	./src/utils/filename.cpp
	./src/structures/grid2d.cpp
)

add_library(sinc_mod SHARED ${SINC_INT_MOD_SRCS})
set_target_properties(sinc_mod PROPERTIES OUTPUT_NAME sinc)

add_library(kdm_mod SHARED ${KDM_MOD_SRCS})
set_target_properties(kdm_mod PROPERTIES OUTPUT_NAME kdm)

add_library(kdm_complex_mod SHARED ${KDM_COMPLEX_MOD_SRCS})
set_target_properties(kdm_complex_mod PROPERTIES OUTPUT_NAME kdm_complex)

add_library(kdmfull_mod SHARED ${KDM_FULL_MOD_SRCS})
set_target_properties(kdmfull_mod PROPERTIES OUTPUT_NAME kdmfull)

add_library(kdmfull2_mod SHARED ${KDM_FULL2_MOD_SRCS})
set_target_properties(kdmfull2_mod PROPERTIES OUTPUT_NAME kdmfull2)

add_library(kdmfull_genconf_mod SHARED ${KDM_FULL_GENCONF_MOD_SRCS})
set_target_properties(kdmfull_genconf_mod PROPERTIES OUTPUT_NAME kdmfull_genconf)

add_library(filter_mod SHARED ${FILTER_MOD_SRCS})
set_target_properties(filter_mod PROPERTIES OUTPUT_NAME filter)

add_library (simple_process_mod SHARED ${SIMPLE_PROCESS_MOD_SRCS})
set_target_properties(simple_process_mod PROPERTIES OUTPUT_NAME simple_process)

add_library (filter_trace_mod SHARED ${FILTER_TRACE_MOD_SRCS})
set_target_properties(filter_trace_mod PROPERTIES OUTPUT_NAME filter_trace)

add_executable(standalone_test ${STANDALONE_SRCS})
target_link_libraries(standalone_test pthread)

install(TARGETS sinc_mod filter_mod simple_process_mod filter_trace_mod
  LIBRARY
  DESTINATION libexec
  COMPONENT modules)

install(TARGETS kdm_mod kdm_complex_mod kdmfull_mod kdmfull2_mod kdmfull_genconf_mod
  LIBRARY
  DESTINATION libexec
  COMPONENT modules)

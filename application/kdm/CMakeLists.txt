cmake_minimum_required(VERSION 2.6.2)

project(KDM_SDPA)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/modules" "$ENV{CMAKE_MODULE_PATH}")

add_subdirectory(pnet)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src
		    ${CMAKE_CURRENT_SOURCE_DIR}/src/include
		    ${CMAKE_CURRENT_SOURCE_DIR}/src/structures
		    ${CMAKE_CURRENT_SOURCE_DIR}/src/filehandler
		    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils
                    ${CMAKE_CURRENT_SOURCE_DIR}/../example
                    ${CMAKE_SOURCE_DIR}/gspc
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

include(SDPAFindBoost)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

find_package(FVM REQUIRED QUIET)
include_directories(${FVM_INCLUDE_DIR})

include_directories(${CMAKE_SOURCE_DIR}/pnet/src)

find_package(UTIL REQUIRED QUIET)
include_directories(${UTIL_INCLUDE_DIR})

find_package(FhgLog REQUIRED QUIET)
include_directories (${FhgLog_INCLUDE_DIR})

find_package(Process REQUIRED QUIET)
include_directories (${PROCESS_INCLUDE_DIR})

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

set (KDM_FULL_MOD_SRCS src/kdmfull.cpp
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
     ./src/determine_size.cpp
     ./src/do_write.cpp
     ./src/do_load.cpp
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

add_library(determine_size SHARED
            ./src/determine_size.cpp
            ./src/utils/swap_bytes.cpp
           )

add_library(do_load SHARED
            ./src/do_load.cpp
            ./src/utils/swap_bytes.cpp
           )

add_library(do_write SHARED
            ./src/do_write.cpp
            ./src/utils/swap_bytes.cpp
           )

add_library(sinc_mod SHARED ${SINC_INT_MOD_SRCS})
set_target_properties(sinc_mod PROPERTIES OUTPUT_NAME sinc)

add_library(kdm_mod SHARED ${KDM_MOD_SRCS})
set_target_properties(kdm_mod PROPERTIES OUTPUT_NAME kdm)
target_link_libraries(kdm_mod sinc_mod)

add_library(kdmfull_mod SHARED ${KDM_FULL_MOD_SRCS})
set_target_properties(kdmfull_mod PROPERTIES OUTPUT_NAME kdmfull)
target_link_libraries(kdmfull_mod sinc_mod)

add_library(filter_mod SHARED ${FILTER_MOD_SRCS})
set_target_properties(filter_mod PROPERTIES OUTPUT_NAME filter)

add_library (simple_process_mod SHARED ${SIMPLE_PROCESS_MOD_SRCS})
set_target_properties(simple_process_mod PROPERTIES OUTPUT_NAME simple_process)

add_library (filter_trace_mod SHARED ${FILTER_TRACE_MOD_SRCS})
target_link_libraries(filter_trace_mod process.shared)
set_target_properties(filter_trace_mod PROPERTIES OUTPUT_NAME filter_trace)

add_executable(standalone_test ${STANDALONE_SRCS})
target_link_libraries(standalone_test pthread)

set (hdrs src/do_load.hpp src/determine_size.hpp src/do_write.hpp)

install(FILES ${hdrs}
  DESTINATION include/seis
  COMPONENT sdk
  )

install(PROGRAMS kdm
  DESTINATION ${SDPA_APPS_DIR}/kdm/bin
  COMPONENT examples
)

install(TARGETS determine_size do_load do_write
  LIBRARY
  DESTINATION libexec/sdpa
  COMPONENT modules)

install(TARGETS sinc_mod filter_mod simple_process_mod filter_trace_mod
  LIBRARY
  DESTINATION ${SDPA_APPS_DIR}/kdm/lib
  COMPONENT modules)

install(TARGETS kdm_mod kdmfull_mod
  LIBRARY
  DESTINATION ${SDPA_APPS_DIR}/kdm/lib
  COMPONENT modules)

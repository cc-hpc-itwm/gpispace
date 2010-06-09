cmake_minimum_required(VERSION 2.6.2)

project(KDM_SDPA)

#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/modules")
#set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})

include_directories(./src
		    ./src/include
		    ./src/structures
		    ./src/filehandler
		    ./src/utils)

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

FIND_PATH(FVM_INCLUDE_DIR Pv4dVM4.h ./lib/FVM/include /opt/cluster/PV-4D.pspro/include)
FIND_LIBRARY(FVM_LIBRARIES Pv4dVM4 ibverbs12 PATH ./lib/FVM/lib64 /opt/cluster/PV-4D.pspro/lib64)

IF (FVM_INCLUDE_DIR AND FVM_LIBRARIES)
  SET(FVM_LIB ${FVM_LIBRARIES})
ENDIF (FVM_INCLUDE_DIR AND FVM_LIBRARIES)

include_directories(${FVM_INCLUDE_DIR})

add_executable(kdm ${SRC})

list(APPEND CMAKE_CXX_FLAGS "-O0 -ggdb")

set_source_files_properties(./src/main.cpp PROPERTIES COMPILE_FLAGS -O0)

target_link_libraries(kdm pthread ibverbs ${FVM_LIB})


# sdpa kdm module
add_definitions(-DSHMEM -DDEBUGALLOC -DDEBUGCOMM  -DSHMEM)

## check for boost
set(Boost_ADDITIONAL_VERSIONS "1.36" "1.36.0" "1.37" "1.37.0" "1.40" "1.40.0")
set(Boost_FIND_QUIETLY false)
set(Boost_USE_STATIC_LIBS OFF)
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

find_path (FVM_PC_INCLUDE_DIR
  NAMES "fvm-pc/pc.hpp"
  HINTS ${FVM_HOME} ENV FVM_HOME
  PATH_SUFFIXES include
)

if (FVM_PC_INCLUDE_DIR)
  include_directories(${FVM_PC_INCLUDE_DIR})
else (FVM_PC_INCLUDE_DIR)
  message(FATAL_ERROR "KDM module requires the FVM process container header")
endif(FVM_PC_INCLUDE_DIR)

find_path (WE_INCLUDE_DIR
  NAMES "we/net.hpp"
  HINTS ${WE_HOME} ENV WE_HOME
  PATH_SUFFIXES include
)

if (WE_INCLUDE_DIR)
  include_directories(${WE_INCLUDE_DIR})
else (WE_INCLUDE_DIR)
  message(FATAL_ERROR "KDM module requires the WE module loader header")
endif(WE_INCLUDE_DIR)

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
target_link_libraries (kdm_mod ${FhgLog_LIBRARY_SHARED} ${Boost_LIBRARIES})
target_link_libraries (kdm_mod sinc_mod)

add_library(fvm_mod SHARED ./src/fvm_mod.cpp)
set_target_properties(fvm_mod PROPERTIES OUTPUT_NAME fvm-test)
target_link_libraries (fvm_mod ${FhgLog_LIBRARY_SHARED} ${Boost_LIBRARIES})

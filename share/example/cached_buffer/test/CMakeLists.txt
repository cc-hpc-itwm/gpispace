

# Compile work functions  
extended_add_library (NAME work_func
  TYPE MODULE
  SOURCES "${CMAKE_SOURCE_DIR}/share/example/cached_buffer/test/implementation.cpp"
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_SOURCE_DIR}/share/example"
  LIBRARIES 
      we-dev
)


math (EXPR nchunks 50)
math (EXPR chunksize 10)
math (EXPR localmem-worker "10<<20")
math (EXPR cachesize-worker "1<<20")
math (EXPR vmemsize-worker "10<<20")


fhg_add_test (NAME share_example_cache
  SOURCES test.cpp
  USE_BOOST
  INCLUDE_DIRECTORIES PRIVATE "${CMAKE_SOURCE_DIR}/share/example"
  ARGS --gspc-home "${CMAKE_INSTALL_PREFIX}"
       --source-directory "${CMAKE_CURRENT_SOURCE_DIR}/../"
       --shared-directory "${SHARED_DIRECTORY_FOR_TESTS}"
       --virtual-memory-startup-timeout 60
       --implementation "$<TARGET_FILE:work_func>"
       --num-chunks ${nchunks}
       --chunk-size ${chunksize}
       --num-bytes ${localmem-worker}
       --cache-bytes ${cachesize-worker}
       --vmem-bytes ${vmemsize-worker}
       --log-host=localhost  --gui-host=localhost --log-port=45095 --gui-port=45096
  LIBRARIES gspc
            test-utilities
            fhg-util
            Boost::program_options
            Boost::filesystem
  DEPENDS work_func
  REQUIRES_INSTALLATION
  REQUIRES_VIRTUAL_MEMORY
  START_SCOPED_RIF
)

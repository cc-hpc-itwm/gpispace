## check for boost
if (NOT BOOST_ROOT)
  set(BOOST_ROOT $ENV{BOOST_ROOT})
endif()

set(Boost_FIND_QUIETLY NO)
set(Boost_USE_STATIC_LIBS ON)
#set(Boost_USE_STATIC_LIBS OFF)
set (COMPONENTS
  thread
  system
  filesystem
  serialization
  program_options
  regex
  iostreams
  date_time
  test_exec_monitor
  unit_test_framework
)
find_package(Boost 1.45 REQUIRED COMPONENTS ${COMPONENTS})
if (Boost_MAJOR_VERSION LESS 1)
  message(FATAL_ERROR "At least Boost 1.45 is required. Found ${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}")
else (Boost_MAJOR_VERSION LESS 1)
  if (Boost_MAJOR_VERSION EQUAL 1)
    if (Boost_MINOR_VERSION LESS 45)
      message(FATAL_ERROR "At least Boost 1.45 is required. Found ${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}")
    endif (Boost_MINOR_VERSION LESS 45)
  endif (Boost_MAJOR_VERSION EQUAL 1)
endif(Boost_MAJOR_VERSION LESS 1)
set(Boost_LIBRARIES "${Boost_REGEX_LIBRARY};${Boost_FILESYSTEM_LIBRARY};${Boost_IOSTREAMS_LIBRARY};${Boost_PROGRAM_OPTIONS_LIBRARY};${Boost_SERIALIZATION_LIBRARY};${Boost_DATE_TIME_LIBRARY};${Boost_THREAD_LIBRARY};${Boost_SYSTEM_LIBRARY}")
set(Boost_UNIT_TEST_LIBRARIES "${Boost_TEST_EXEC_MONITOR_LIBRARY};${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}")
get_filename_component (BOOST_ROOT "${Boost_INCLUDE_DIR}" PATH CACHE)

if (Boost_MAJOR_VERSION EQUAL 1)
  if (Boost_MINOR_VERSION EQUAL 48)
    message(FATAL_ERROR "Incompatible Boost 1.48 detected")
  endif ()
endif ()

if (Boost_MAJOR_VERSION EQUAL 1)
  if (Boost_MINOR_VERSION GREATER 51)
    # Boost.Thread now throws when preconditions are not met.
    # This is the case when calling join() in some tests, while
    # joinable() == false.  This define is only a hacky workaround and
    # the precondition shall be checked everywhere, where needed.
    add_definitions (-DBOOST_THREAD_TRROW_IF_PRECONDITION_NOT_SATISFIED)
  endif()
endif()

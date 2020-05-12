extended_add_library (NAME mmgr
  SOURCES "dtmmgr.cpp"
          "tmmgr.cpp"
  LIBRARIES Boost::base
)

add_subdirectory (tests)

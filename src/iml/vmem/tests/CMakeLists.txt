fhg_add_test (NAME mmgr_dtmmgr
  SOURCES dtmmgr.cpp
  USE_BOOST
  LIBRARIES mmgr fhg-util
)
fhg_add_test (NAME mmgr_tmmgr
  SOURCES tmmgr.cpp
  USE_BOOST
  LIBRARIES mmgr fhg-util
)
fhg_add_test (NAME mmgr_perftmmgr
  SOURCES perftmmgr.cpp
  USE_BOOST PERFORMANCE_TEST
  LIBRARIES mmgr fhg-util
)

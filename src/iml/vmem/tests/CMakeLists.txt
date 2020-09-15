fhg_add_test (NAME iml-vmem_netdev_id
  SOURCES "netdev_id.cpp"
  LIBRARIES IML::vmem-client
  USE_BOOST
)

fhg_add_test (NAME iml-mmgr_dtmmgr
  SOURCES "dtmmgr.cpp"
  USE_BOOST
  LIBRARIES IML::mmgr IML::fhg-util
)
fhg_add_test (NAME iml-mmgr_tmmgr
  SOURCES "tmmgr.cpp"
  USE_BOOST
  LIBRARIES IML::mmgr IML::fhg-util
)
fhg_add_test (NAME iml-mmgr_perftmmgr
  SOURCES "perftmmgr.cpp"
  USE_BOOST PERFORMANCE_TEST
  LIBRARIES IML::mmgr IML::fhg-util
)

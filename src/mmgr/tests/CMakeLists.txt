# mirko.rahn@itwm.fraunhofer.de

fhg_add_test (dtmmgr.cpp BOOST_UNIT_TEST PROJECT mmgr LINK_LIBRARIES mmgr)
fhg_add_test (fseg.cpp PROJECT mmgr LINK_LIBRARIES mmgr)
fhg_add_test (heap.cpp BOOST_UNIT_TEST PROJECT mmgr LINK_LIBRARIES mmgr)
fhg_add_test (ostab.cpp PROJECT mmgr LINK_LIBRARIES mmgr)
fhg_add_test (smap.cpp PROJECT mmgr LINK_LIBRARIES mmgr)
fhg_add_test (tmmgr.cpp PROJECT mmgr LINK_LIBRARIES mmgr)
fhg_add_test (trie.cpp BOOST_UNIT_TEST PROJECT mmgr LINK_LIBRARIES mmgr)

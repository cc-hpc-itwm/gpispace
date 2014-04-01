add_subdirectory(use_sequence)

fhg_add_test (verify.cpp
  PROJECT pnetv
  BOOST_UNIT_TEST
  LINK_LIBRARIES fhg-util pnetv-jpna pnetv-jpn pnet
)

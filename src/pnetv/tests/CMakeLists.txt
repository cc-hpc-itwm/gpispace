add_subdirectory(use_sequence)

fhg_add_test (NAME pnetv_verify
  SOURCES verify.cpp
  USE_BOOST
  LIBRARIES fhg-util pnetv-jpna pnetv-jpn pnet
)

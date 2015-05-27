extended_add_library (NAME pnetv-jpn
  SOURCES "jpn/Marking.cpp"
          "jpn/analysis/Backtrack.cpp"
          "jpn/analysis/StateSpace.cpp"
)
extended_add_library (NAME pnetv-jpna
  SOURCES "jpna/Parsing.cpp"
          "jpna/PetriNet.cpp"
          "jpna/Verification.cpp"
  LIBRARIES fhg-util
)

add_subdirectory (tests)

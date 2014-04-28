add_library (pnetv-jpn STATIC
    jpn/Marking.cpp
    jpn/analysis/Backtrack.cpp
    jpn/analysis/StateSpace.cpp
)
add_library(pnetv-jpna STATIC
    jpna/Parsing.cpp
    jpna/PetriNet.cpp
    jpna/Verification.cpp
)
target_link_libraries (pnetv-jpna fhg-util)

	add_subdirectory(tests)

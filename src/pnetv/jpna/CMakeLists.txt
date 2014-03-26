add_library(pnetv-jpna STATIC
    Parsing.cpp
    PetriNet.cpp
    Verification.cpp
)
target_link_libraries (pnetv-jpna fhg-util)

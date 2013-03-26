set(PNETV_JPNA_SOURCES
    Parsing.cpp
    Parsing.h
    PetriNet.cpp
    PetriNet.h
    Place.h
    Transition.h
    Types.h
    Verification.cpp
    Verification.h
    VerificationResult.cpp
    VerificationResult.h
)

add_library(pnetv-jpna STATIC ${PNETV_JPNA_SOURCES})
target_link_libraries (pnetv-jpna fhg-util)

# vim:set et sts=4 sw=4 nospell:

set(PNETV_PNETA_SOURCES
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

add_library(pnetv-pneta STATIC ${PNETV_PNETA_SOURCES})

# vim:set et sts=4 sw=4 nospell:

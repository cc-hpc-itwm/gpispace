set(PNETV_JPN_SOURCES
    config.h
    Marking.cpp
    Marking.h
    PlaceMarking.h
    PlaceMarking.cpp
    Transition.h
    Types.h
    analysis/Backtrack.cpp
    analysis/Backtrack.h
    analysis/State.h
    analysis/StateSpace.cpp
    analysis/StateSpace.h
    common/Foreach.h
    common/ExtendedInteger.h
    common/Printable.h
    common/Printable.cpp
    common/PrintRange.h
    common/Unreachable.h
)

add_library(pnetv-jpn STATIC ${PNETV_JPN_SOURCES})

# vim:set et sts=4 sw=4 nospell:

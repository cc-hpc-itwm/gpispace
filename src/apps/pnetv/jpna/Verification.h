#pragma once

#include "VerificationResult.h"

namespace jpna {

class PetriNet;

VerificationResult verify(const PetriNet &petriNet);

} // namespace jpna

/* vim:set et sts=4 sw=4: */

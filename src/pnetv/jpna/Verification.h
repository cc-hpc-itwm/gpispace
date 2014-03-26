#pragma once

#include "VerificationResult.h"

namespace jpna {

class PetriNet;

VerificationResult verify(const PetriNet &petriNet);

} // namespace jpna

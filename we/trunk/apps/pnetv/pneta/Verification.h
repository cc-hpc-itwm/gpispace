#pragma once

#include "VerificationResult.h"

namespace pneta {

class PetriNet;

VerificationResult verify(const PetriNet &petriNet);

} // namespace pneta

/* vim:set et sts=4 sw=4: */

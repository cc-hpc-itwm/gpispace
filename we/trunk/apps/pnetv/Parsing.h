#pragma once

#include <string>
#include <vector>

namespace pnetv {

class PetriNet;

void parse(const char *filename, std::vector<PetriNet> &petriNets);

} // namespace pnetv

/* vim:set et sts=4 sw=4: */

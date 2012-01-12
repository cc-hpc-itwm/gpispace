#pragma once

#include <string>
#include <vector>

namespace pnetv {

class PetriNet;

/**
 * Parse a file with an activity_t instance stored into our abstraction over compiled workflows.
 *
 * \param[in] filename Input file name.
 * \param[out] petriNets Where to store the Petri nets.
 */
void parse(const char *filename, std::vector<PetriNet> &petriNets);

} // namespace pnetv

/* vim:set et sts=4 sw=4: */

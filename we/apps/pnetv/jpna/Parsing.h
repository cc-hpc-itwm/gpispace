#pragma once

#include <string>
#include <vector>

#include <boost/ptr_container/ptr_vector.hpp>

namespace jpna {

class PetriNet;

/**
 * Parse a file with an activity_t instance stored into our abstraction over compiled workflows.
 *
 * \param[in] filename Input file name.
 * \param[out] petriNets Where to store the Petri nets.
 */
void parse(const char *filename, boost::ptr_vector<PetriNet> &petriNets);

/**
 * Parse a file with an activity_t instance stored into our abstraction over compiled workflows.
 *
 * \param[in] filename Input file name.
 * \param[in] in Input file stream.
 * \param[out] petriNets Where to store the Petri nets.
 */
void parse(const char *filename, std::istream &in, boost::ptr_vector<PetriNet> &petriNets);

} // namespace jpna

/* vim:set et sts=4 sw=4: */

#pragma once

#include <string>

#include <boost/optional.hpp>

#include "Types.h"

namespace pnetv {

/**
 * Place from a workflow abstracted from unnecessary details.
 */
class Place {
    std::string name_; ///< Name of the transition.
    boost::optional<TokenCount> capacity_; ///< Capacity, if any.

    public:

    /**
     * Construction.
     *
     * \param name Name of the place.
     * \param capacity Capacity of the place, if any.
     */
    Place(const std::string &name, boost::optional<TokenCount> capacity):
        name_(name), capacity_(capacity)
    {}
    
    /**
     * \return Name of the place.
     */
    const std::string &name() const { return name_; }

    /**
     * \return Capacity of the place.
     */
    const boost::optional<TokenCount> &capacity() const { return capacity_; }
};

} // namespace pnetv

/* vim:set et sts=4 sw=4: */

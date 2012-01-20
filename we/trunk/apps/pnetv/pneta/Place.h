#pragma once

#include <string>

#include <boost/optional.hpp>

#include "Types.h"

namespace pneta {

/**
 * Place from a workflow abstracted from unnecessary details.
 */
class Place {
    PlaceId id_; ///< Id of the place.
    std::string name_; ///< Name of the transition.
    boost::optional<TokenCount> capacity_; ///< Capacity, if any.
    TokenCount initialMarking_; ///< Initial marking.

    public:

    /**
     * Constructor.
     *
     * \param id Id.
     */
    Place(PlaceId id): id_(id), initialMarking_(0) {}
    
    /**
     * \return Id of the transition.
     */
    PlaceId id() const { return id_; }

    /**
     * \return Name of the place.
     */
    const std::string &name() const { return name_; }

    /**
     * Sets the name of the transition.
     *
     * \param name New name.
     */
    void setName(const std::string &name) { name_ = name; }

    /**
     * \return Capacity of the place.
     */
    const boost::optional<TokenCount> &capacity() const { return capacity_; }

    /**
     * Sets capacity of the place.
     *
     * \param count Maximal number of tokens allowed.
     */
    void setCapacity(boost::optional<TokenCount> count) { capacity_ = count; }

    /**
     * \return Initial marking of this place.
     */
    TokenCount initialMarking() const { return initialMarking_; }

    /**
     * Sets initial marking of the place.
     *
     * \param count Initial token count.
     */
    void setInitialMarking(TokenCount count) { initialMarking_ = count; }
};

} // namespace pneta

/* vim:set et sts=4 sw=4: */

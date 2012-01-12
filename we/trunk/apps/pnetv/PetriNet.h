#pragma once

#include <boost/unordered_map.hpp>

#include "Place.h"
#include "Transition.h"

namespace pnetv {

/**
 * Petri Net representing a workflow with unnecessary details being abstracted away.
 */
class PetriNet {
    std::vector<Transition> transitions_; ///< Transitions.
    std::vector<Place> places_; ///< Places.

    public:

    /**
     * \return All the transitions in the Petri net.
     */
    const std::vector<Transition> &transitions() const { return transitions_; }

    /**
     * Adds a transition to the Petri net.
     *
     * \param transition Transition.
     */
    void addTransition(const Transition &transition) { transitions_.push_back(transition); }

    /**
     * \param id Id of the transitions.
     *
     * \return Transition with given id.
     */
    Transition &getTransition(TransitionId id) { return transitions_[id]; }

    /**
     * \param id Id of the transitions.
     *
     * \return Transition with given id.
     */
    const Transition &getTransition(TransitionId id) const { return transitions_[id]; }

    /**
     * \return All the places in the Petri net.
     */
    const std::vector<Place> &places() const { return places_; }

    /**
     * Adds a place to the Petri net.
     *
     * \param place Place.
     */
    void addPlace(const Place &place) { places_.push_back(place); }

    /**
     * \param id Id of the place.
     *
     * \return Place with given id.
     */
    Place &getPlace(PlaceId id) { return places_[id]; }

    /**
     * \param id Id of the place.
     *
     * \return Place with given id.
     */
    const Place &getPlace(PlaceId id) const { return places_[id]; }
};

} // namespace pnetv

/* vim:set et sts=4 sw=4: */

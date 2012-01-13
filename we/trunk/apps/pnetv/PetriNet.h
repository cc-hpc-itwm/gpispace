#pragma once

#include <boost/unordered_map.hpp>

#include "Place.h"
#include "Transition.h"

namespace pnetv {

/**
 * Petri Net representing a workflow with unnecessary details being abstracted away.
 */
class PetriNet {
    std::string name_; ///< Name of the Petri net.
    std::vector<Transition> transitions_; ///< Transitions.
    std::vector<Place> places_; ///< Places.

    public:

    /**
     * Constructor.
     *
     * \param name Name of the Petri net.
     */
    PetriNet(const std::string &name) { name_ = name; }

    /**
     * \return Name of the Petri net.
     */
    const std::string &name() const { return name_; }

    /**
     * \return All the transitions in the Petri net.
     */
    const std::vector<Transition> &transitions() const { return transitions_; }

    /**
     * Adds a transition to the Petri net.
     *
     * \param transition Transition.
     */
    TransitionId addTransition(const Transition &transition) {
        transitions_.push_back(transition);
        return transitions_.size() - 1;
    }

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
    PlaceId addPlace(const Place &place) {
        places_.push_back(place);
        return places_.size() - 1;
    }

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

    /**
     * Adds an input arc from a place to a transition.
     *
     * \param transitionId Id of a transition.
     * \param placeId      Id of a place.
     */
    void addInputArc(TransitionId transitionId, PlaceId placeId) {
        getTransition(transitionId).addInputPlace(placeId);
    }

    /**
     * Adds an output arc from a transition to a place.
     *
     * \param transitionId Id of a transition.
     * \param placeId      Id of a place.
     */
    void addOutputArc(TransitionId transitionId, PlaceId placeId) {
        getTransition(transitionId).addOutputPlace(placeId);
    }
};

} // namespace pnetv

/* vim:set et sts=4 sw=4: */

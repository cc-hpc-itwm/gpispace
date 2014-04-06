#pragma once

#include "Place.h"
#include "Transition.h"

#include <we/type/id.hpp>

#include <unordered_map>

namespace jpna {

/**
 * Petri Net representing a workflow with unnecessary details being abstracted away.
 */
class PetriNet {
    std::string name_; ///< Name of the Petri net.
    std::vector<Transition *> transitions_; ///< Transitions.
    std::vector<Place *> places_; ///< Places.

    public:

    /**
     * Constructor.
     *
     * \param name Name of the Petri net.
     */
    PetriNet(const std::string &name) { name_ = name; }

    /**
     * Destructor.
     */
    ~PetriNet();

    /**
     * \return Name of the Petri net.
     */
    const std::string &name() const { return name_; }

    /**
     * \return Newly created transition owned by the Petri net.
     */
    Transition *createTransition();

    /**
     * \return All the transitions in the Petri net.
     */
    const std::vector<Transition *> &transitions() { return transitions_; }

    /**
     * \return All the transitions in the Petri net.
     */
    const std::vector<const Transition *> &transitions() const { return reinterpret_cast<const std::vector<const Transition *> &>(transitions_); }

    /**
     * \param id Id of the transitions.
     *
     * \return Transition with given id.
     */
    Transition *getTransition(we::transition_id_type id) { return transitions_[id.value()]; }

    /**
     * \param id Id of the transitions.
     *
     * \return Transition with given id.
     */
    const Transition *getTransition(we::transition_id_type id) const { return transitions_[id.value()]; }
    /**
     * \return Newly created place owned by the Petri net.
     */
    Place *createPlace (std::string const&, TokenCount);

    /**
     * \return All the places in the Petri net.
     */
    const std::vector<Place *> &places() { return places_; }

    /**
     * \return All the places in the Petri net.
     */
    const std::vector<const Place *> &places() const { return reinterpret_cast<const std::vector<const Place *> &>(places_); }

    /**
     * \param id Valid id of the place.
     *
     * \return Place with given id.
     */
    Place *getPlace(we::place_id_type id) { return places_[id.value()]; }

    /**
     * \param id Id of the place.
     *
     * \return Place with given id.
     */
    const Place *getPlace(we::place_id_type id) const { return places_[id.value()]; }

    void increment_token_count (we::place_id_type place_id) const
    {
      ( *std::find_if
        ( places_.begin(), places_.end()
        , [&place_id](Place* place) { return place->id() == place_id; }
        )
      )->increment_token_count();
    }
};

} // namespace jpna

#pragma once

#include <string>
#include <vector>

#include <we/type/id.hpp>

#include "Types.h"

namespace jpna {

class Place;

/**
 * Transition from a workflow abstracted from unnecessary details.
 */
class Transition {
    TransitionId id_; ///< Id of the transition.
    std::string name_; ///< Name of the transition.
    std::vector<const Place *> inputPlaces_; ///< Ids of input places.
    std::vector<const Place *> outputPlaces_; ///< Ids of output places.
    bool conditionAlwaysTrue_; ///< True iff transition's condition is constant true.
    petri_net::prio_t priority_; ///< Priority.

    public:

    /**
     * Constructor.
     *
     * \param id Id.
     */
    Transition(TransitionId id): id_(id), conditionAlwaysTrue_(true), priority_(0) {}

    /**
     * \return Id of the transition.
     */
    TransitionId id() const { return id_; }

    /**
     * Name of the transition.
     */
    const std::string &name() const { return name_; }

    /**
     * Sets the name of the transition.
     *
     * \param name New name.
     */
    void setName(const std::string &name) { name_ = name; }

    /**
     * \return True iff transition's condition is constant true.
     */
    bool conditionAlwaysTrue() const { return conditionAlwaysTrue_; }

    /**
     * Sets whether condition is always true.
     *
     * \param value True iff the condition is always true.
     */
    void setConditionAlwaysTrue(bool value) { conditionAlwaysTrue_ = value; }

    /**
     * \return Priority of the transition.
     * Transitions with lower priority don't fire unless there are enabled transitions with higher priority.
     */
    petri_net::prio_t priority() const { return priority_; }

    /**
     * Sets the transition's priority.
     *
     * \param[in] priority New priority.
     */
    void setPriority(petri_net::prio_t priority) { priority_ = priority; }

    /**
     * Adds input place.
     *
     * \param place Place.
     */
    void addInputPlace(const Place *place) { inputPlaces_.push_back(place); }

    /**
     * \return Ids of input places.
     */
    const std::vector<const Place *> &inputPlaces() const { return inputPlaces_; }

    /**
     * Adds output place.
     *
     * \param place Place.
     */
    void addOutputPlace(const Place *place) { outputPlaces_.push_back(place); }

    /**
     * \return Ids of output places.
     */
    const std::vector<const Place *> &outputPlaces() const { return outputPlaces_; }
};

} // namespace jpna

/* vim:set et sts=4 sw=4: */

#pragma once

#include <string>
#include <vector>

#include "Types.h"

namespace pneta {

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
    bool firesFinitely_; ///< True iff transition fires finite number of times.

    public:

    /**
     * Constructor.
     *
     * \param id Id.
     */
    Transition(TransitionId id): id_(id), conditionAlwaysTrue_(true), firesFinitely_(false) {}

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
     * \return True iff transition fires finite number of times.
     */
    bool firesFinitely() const { return firesFinitely_; }

    /**
     * \param value True iff transition fires finite number of times.
     */
    void setFiresFinitely(bool value) { firesFinitely_ = value; }

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

} // namespace pneta

/* vim:set et sts=4 sw=4: */

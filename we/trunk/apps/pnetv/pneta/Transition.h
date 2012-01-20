#pragma once

#include <string>
#include <vector>

#include "Types.h"

namespace pneta {

/**
 * Transition from a workflow abstracted from unnecessary details.
 */
class Transition {
    std::string name_; ///< Name of the transition.
    std::vector<PlaceId> inputPlaces_; ///< Ids of input places.
    std::vector<PlaceId> outputPlaces_; ///< Ids of output places.
    bool conditionIsAlwaysTrue_; ///< True iff transition's condition is constant true.

    public:

    /**
     * Constructor.
     *
     * \param name Name of the transition.
     * \param conditionIsAlwaysTrue Whether transition's condition is constant true.
     */
    Transition(const std::string &name, bool conditionIsAlwaysTrue = true):
        name_(name), conditionIsAlwaysTrue_(conditionIsAlwaysTrue)
    { return; }

    /**
     * Name of the transition.
     */
    const std::string &name() const { return name_; }

    /**
     * \return True iff transition's condition is constant true.
     */
    bool conditionIsAlwaysTrue() const { return conditionIsAlwaysTrue_; }

    /**
     * Adds input place.
     *
     * \param id Place id.
     */
    void addInputPlace(PlaceId id) { inputPlaces_.push_back(id); }

    /**
     * \return Ids of input places.
     */
    const std::vector<PlaceId> &inputPlaces() const { return inputPlaces_; }

    /**
     * Adds output place.
     *
     * \param id Place id.
     */
    void addOutputPlace(PlaceId id) { outputPlaces_.push_back(id); }

    /**
     * \return Ids of output places.
     */
    const std::vector<PlaceId> &outputPlaces() const { return outputPlaces_; }
};

} // namespace pneta

/* vim:set et sts=4 sw=4: */

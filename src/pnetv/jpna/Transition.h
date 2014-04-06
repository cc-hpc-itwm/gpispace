#pragma once

#include "Types.h"

#include <we/type/id.hpp>

#include <string>
#include <vector>

namespace jpna {

/**
 * Transition from a workflow abstracted from unnecessary details.
 */
class Transition {
    we::transition_id_type id_; ///< Id of the transition.
    std::string name_; ///< Name of the transition.
    std::vector<we::place_id_type> inputPlaces_; ///< Ids of input places.
    std::vector<we::place_id_type> outputPlaces_; ///< Ids of output places.
    bool conditionAlwaysTrue_; ///< True iff transition's condition is constant true.
    we::priority_type priority_; ///< Priority.

    public:

    /**
     * Constructor.
     *
     * \param id Id.
     */
    Transition(we::transition_id_type id): id_(id), conditionAlwaysTrue_(true), priority_(0) {}

    /**
     * \return Id of the transition.
     */
    we::transition_id_type id() const { return id_; }

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
    we::priority_type priority() const { return priority_; }

    /**
     * Sets the transition's priority.
     *
     * \param[in] priority New priority.
     */
    void setPriority(we::priority_type priority) { priority_ = priority; }

    void addInputPlace (we::place_id_type place_id)
    {
      inputPlaces_.push_back (place_id);
    }

    /**
     * \return Ids of input places.
     */
    const std::vector<we::place_id_type> &inputPlaces() const { return inputPlaces_; }

    void addOutputPlace (we::place_id_type place_id)
    {
      outputPlaces_.push_back (place_id);
    }

    /**
     * \return Ids of output places.
     */
    const std::vector<we::place_id_type> &outputPlaces() const { return outputPlaces_; }
};

} // namespace jpna

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
    Transition ( we::transition_id_type id
               , std::string const& name
               , bool condition_always_true
               , we::priority_type priority
               )
      : id_ (id)
      , name_ (name)
      , conditionAlwaysTrue_ (condition_always_true)
      , priority_ (priority)
    {}

    /**
     * \return Id of the transition.
     */
    we::transition_id_type id() const { return id_; }

    /**
     * Name of the transition.
     */
    const std::string &name() const { return name_; }

    /**
     * \return True iff transition's condition is constant true.
     */
    bool conditionAlwaysTrue() const { return conditionAlwaysTrue_; }

    /**
     * \return Priority of the transition.
     * Transitions with lower priority don't fire unless there are enabled transitions with higher priority.
     */
    we::priority_type priority() const { return priority_; }

    void addInputPlace (we::place_id_type place_id)
    {
      inputPlaces_.emplace_back (place_id);
    }

    /**
     * \return Ids of input places.
     */
    const std::vector<we::place_id_type> &inputPlaces() const { return inputPlaces_; }

    void addOutputPlace (we::place_id_type place_id)
    {
      outputPlaces_.emplace_back (place_id);
    }

    /**
     * \return Ids of output places.
     */
    const std::vector<we::place_id_type> &outputPlaces() const { return outputPlaces_; }
};

} // namespace jpna

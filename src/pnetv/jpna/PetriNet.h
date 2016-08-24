#pragma once

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
    std::unordered_map<we::place_id_type, TokenCount> _token_count;

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
    Transition *createTransition ( std::string const& name
                                 , bool condition_always_true
                                 , we::priority_type priority
                                 );

    /**
     * \return All the transitions in the Petri net.
     */
    std::vector<Transition *> transitions() const { return transitions_; }

    /**
     * \param id Id of the transitions.
     *
     * \return Transition with given id.
     */
    const Transition *getTransition(we::transition_id_type id) const { return transitions_[id.value()]; }
    /**
     * \return Newly created place owned by the Petri net.
     */
    we::place_id_type createPlace (TokenCount);

    void increment_token_count (we::place_id_type place_id)
    {
      ++_token_count[place_id];
    }

    std::unordered_map<we::place_id_type, TokenCount> const& token_count() const
    {
      return _token_count;
    }
};

} // namespace jpna

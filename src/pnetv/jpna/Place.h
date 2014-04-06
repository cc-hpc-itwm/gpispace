#pragma once

#include "Types.h"

#include <we/type/id.hpp>

#include <boost/optional.hpp>

#include <string>

namespace jpna {

/**
 * Place from a workflow abstracted from unnecessary details.
 */
class Place {
    we::place_id_type id_; ///< Id of the place.
    std::string name_; ///< Name of the transition.
    TokenCount initialMarking_; ///< Initial marking.

    public:

    Place ( we::place_id_type id
          , std::string const& name
          , TokenCount initial_marking
          )
      : id_ (id)
      , name_ (name)
      , initialMarking_ (initial_marking)
    {}

    /**
     * \return Id of the place.
     */
    we::place_id_type id() const { return id_; }

    /**
     * \return Initial marking of this place.
     */
    TokenCount initialMarking() const { return initialMarking_; }

    void increment_token_count()
    {
      ++initialMarking_;
    }
};

} // namespace jpna

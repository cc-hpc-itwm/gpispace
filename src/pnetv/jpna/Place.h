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

    public:

    Place ( we::place_id_type id
          , std::string const& name
          )
      : id_ (id)
      , name_ (name)
    {}

    /**
     * \return Id of the place.
     */
    we::place_id_type id() const { return id_; }
};

} // namespace jpna

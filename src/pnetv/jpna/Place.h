#pragma once

#include "Types.h"

#include <we/type/id.hpp>

#include <boost/optional.hpp>

namespace jpna {

/**
 * Place from a workflow abstracted from unnecessary details.
 */
class Place {
    we::place_id_type id_; ///< Id of the place.

    public:

    Place ( we::place_id_type id
          )
      : id_ (id)
    {}

    /**
     * \return Id of the place.
     */
    we::place_id_type id() const { return id_; }
};

} // namespace jpna

#pragma once

#include <string>

#include <boost/optional.hpp>

#include "Types.h"

#include <we/type/id.hpp>

namespace jpna {

/**
 * Place from a workflow abstracted from unnecessary details.
 */
class Place {
    we::place_id_type id_; ///< Id of the place.
    std::string name_; ///< Name of the transition.
    TokenCount initialMarking_; ///< Initial marking.

    public:

    /**
     * Constructor.
     *
     * \param id Id.
     */
    Place(we::place_id_type id): id_(id), initialMarking_(0) {}

    /**
     * \return Id of the transition.
     */
    we::place_id_type id() const { return id_; }

    /**
     * \return Name of the place.
     */
    const std::string &name() const { return name_; }

    /**
     * Sets the name of the transition.
     *
     * \param name New name.
     */
    void setName(const std::string &name) { name_ = name; }

    /**
     * \return Initial marking of this place.
     */
    TokenCount initialMarking() const { return initialMarking_; }

    /**
     * Sets initial marking of the place.
     *
     * \param count Initial token count.
     */
    void setInitialMarking(TokenCount count) { initialMarking_ = count; }
};

} // namespace jpna

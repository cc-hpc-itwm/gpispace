#pragma once

#include <jpn/config.h>
#include <jpn/Marking.h>

namespace jpn {

/**
 * Transition.
 */
class Transition {
    TransitionId id_; ///< Transition id.
    Marking input_; ///< Input of the transition.
    Marking output_; ///< Output of the transition.
    Marking effect_; ///< Effect of the transition (output - input).
    bool firesFinitely_; ///< True iff transition fires finite number of times.

    public:

    /**
     * Class constructor.
     */
    Transition(TransitionId id, const Marking &input, const Marking &output, bool firesFinitely = false):
        id_(id), input_(input), output_(output), effect_(output - input), firesFinitely_(firesFinitely)
    {}

    /**
     * \return Transition id.
     */
    TransitionId id() const { return id_; }

    /**
     * \return Input of the transition.
     */
    const Marking &input() const { return input_; }

    /**
     * \return Output of the transition.
     */
    const Marking &output() const { return output_; }

    /**
     * \return Effect of the transition (output - input).
     */
    const Marking &effect() const { return effect_; }

    /**
     * \param[in] marking Marking.
     *
     * \return True iff the transition can fire in given marking.
     */
    bool canFire(const Marking &marking) const { return input() <= marking; }

    /**
     * \param[in] marking Marking.
     *
     * \return Marking which is the result of firing the transition in given marking.
     */
    Marking fire(const Marking &marking) const { return marking + effect(); }

    /**
     * \return True iff transition fires finite number of times.
     */
    bool firesFinitely() const { return firesFinitely_; }
};

} // namespace jpn

/* vim:set et sts=4 sw=4: */

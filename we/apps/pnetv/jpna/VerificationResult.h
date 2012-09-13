#pragma once

#include <cassert>
#include <vector>
#include <fhg/assert.hpp>

#include <jpn/common/Printable.h>

namespace jpna {

class Transition;

/**
 * Result of a verification.
 */
class VerificationResult: public jpn::Printable {
    public:

    /**
     * Verdict.
     */
    enum Result {
        TERMINATES, ///< Terminates.
        LOOPS,      ///< Definitely loops.
        MAYBE_LOOPS ///< Maybe loops (trace contains transitions with non-trivial conditions).
    };

    private:

    Result result_; ///< Verdict.
    std::vector<const Transition *> init_; ///< Trace leading to the loop.
    std::vector<const Transition *> loop_; ///< Trace realising one iteration of the loop.

    public:

    /**
     * Constructor for verdicts not requiring traces.
     *
     * \param result Verdict.
     */
    VerificationResult(Result result):
        result_(result)
    {
        assert(result == TERMINATES);
    }

    /**
     * Constructor for verdicts requiring traces.
     *
     * \param result Verdict.
     * \param init Trace leading to the loop.
     * \param loop Trace realising one iteration of the loop.
     */
    VerificationResult(Result result, const std::vector<const Transition *> &init, const std::vector<const Transition *> &loop):
        result_(result), init_(init), loop_(loop)
    {
        assert(result != TERMINATES);
    }

    /**
     * \return Verdict.
     */
    Result result() const { return result_; }

    /**
     * \return Trace leading to the loop.
     */
    const std::vector<const Transition *> &init() const { return init_; }

    /**
     * \return Trace realising one iteration of the loop.
     */
    const std::vector<const Transition *> &loop() const { return loop_; }

    virtual void print(std::ostream &out) const;
};

} // namespace jpna

/* vim:set et sts=4 sw=4: */

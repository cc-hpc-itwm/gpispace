#pragma once

#include <cassert>
#include <vector>

#include <jpn/common/Printable.h>

namespace pneta {

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
        TERMINATES,      ///< Terminates.
        UNBOUNDED,       ///< Definitely unbounded.
        MAYBE_UNBOUNDED, ///< Maybe unbounded (trace contains transitions with non-trivial conditions).
        INFINITE,        ///< Definitely has infinite loop.
        MAYBE_INFINITE,  ///< Maybe has an infinite loop (trace contains transitions with non-trivial conditions).
    };

    private:

    Result result_; ///< Verdict.
    std::vector<const Transition *> trace_; ///< Trace.

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
     * \param trace Trace, a sequence of transition ids.
     */
    VerificationResult(Result result, const std::vector<const Transition *> &trace):
        result_(result), trace_(trace)
    {
        assert(result != TERMINATES);
    }

    /**
     * \return Verdict.
     */
    Result result() const { return result_; }

    /**
     * \return Trace.
     */
    const std::vector<const Transition *> &trace() const { return trace_; }

    virtual void print(std::ostream &out) const;
};

} // namespace pneta

/* vim:set et sts=4 sw=4: */

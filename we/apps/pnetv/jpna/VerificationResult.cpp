#include "VerificationResult.h"

#include <jpn/common/Foreach.h>
#include <jpn/common/Unreachable.h>

#include "Transition.h"

namespace jpna {

void VerificationResult::print(std::ostream &out) const {
    out << "(";
    switch (result()) {
        case TERMINATES:
            out << "TERMINATES";
            break;
        case LOOPS:
            out << "LOOPS";
            break;
        case MAYBE_LOOPS:
            out << "MAYBE_LOOPS";
            break;
        default:
            jpn::unreachable();
    }
    if (result() != TERMINATES) {
        out << ", init:{";

        bool comma = false;
        FOREACH (const Transition *transition, init()) {
            if (comma) {
                out << ", ";
            } else {
                comma = true;
            }
            out << "`" << transition->name() << "'";
        }
        out << "}, loop:{";

        comma = false;
        FOREACH (const Transition *transition, loop()) {
            if (comma) {
                out << ", ";
            } else {
                comma = true;
            }
            out << "`" << transition->name() << "'";
        }
        out << "}";
    }
    out << ")";
}

} // namespace jpna

/* vim:set et sts=4 sw=4: */

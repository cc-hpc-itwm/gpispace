#include "VerificationResult.h"

#include <jpn/common/Foreach.h>
#include <jpn/common/Unreachable.h>

#include "Transition.h"

namespace pneta {

void VerificationResult::print(std::ostream &out) const {
    out << "(";
    switch (result()) {
        case TERMINATES:
            out << "TERMINATES";
            break;
        case UNBOUNDED:
            out << "UNBOUNDED";
            break;
        case MAYBE_UNBOUNDED:
            out << "MAYBE_UNBOUNDED";
            break;
        case INFINITE:
            out << "INFINITE";
            break;
        case MAYBE_INFINITE:
            out << "MAYBE_INFINITE";
            break;
        default:
            jpn::unreachable();
    }
    if (result() != TERMINATES) {
        out << ", {";

	bool comma = false;
	foreach (const Transition *transition, trace()) {
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

} // namespace pneta

/* vim:set et sts=4 sw=4: */

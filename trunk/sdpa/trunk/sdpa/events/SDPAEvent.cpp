#include "SDPAEvent.hpp"

using namespace sdpa::events;

SDPAEvent::SDPAEvent(const address_t & from, const address_t &to)
    : from_(from), to_(to) {

}

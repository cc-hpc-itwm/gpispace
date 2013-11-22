#include "SDPAEvent.hpp"

using namespace sdpa::events;

SDPAEvent::SDPAEvent (const address_t & a_from, const address_t &a_to)
 : from_ (a_from)
 , to_ (a_to)
{}

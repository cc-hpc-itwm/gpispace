#include "SDPAEvent.hpp"
#include <sdpa/events/id_generator.hpp>

using namespace sdpa::events;

SDPAEvent::SDPAEvent(const address_t & a_from, const address_t &a_to)
 : from_(a_from), to_(a_to)
 , id_ (id_generator::instance().next())
{ }

SDPAEvent::SDPAEvent(const address_t & a_from, const address_t &a_to, const message_id_type &mid)
 : from_(a_from), to_(a_to), id_(mid)
{ }

SDPAEvent::SDPAEvent(const SDPAEvent &other)
  : IEvent(), from_(other.from()), to_(other.to()), id_(other.id())
{ }

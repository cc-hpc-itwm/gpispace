#include "SDPAEvent.hpp"
#include <sdpa/id_generator.hpp>

using namespace sdpa::events;

namespace
{
  struct msg_id_tag
  {
    static const char *name ()
    {
      return "msg";
    }
  };
}

SDPAEvent::SDPAEvent (const address_t & a_from, const address_t &a_to)
 : from_ (a_from)
 , to_ (a_to)
 , id_ (id_generator<msg_id_tag>::instance().next())
{}

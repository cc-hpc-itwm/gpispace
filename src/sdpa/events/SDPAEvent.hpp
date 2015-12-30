#pragma once

#include <sdpa/events/EventHandler.hpp>

#include <fhgcom/header.hpp>

#include <boost/serialization/access.hpp>
#include <boost/shared_ptr.hpp>

namespace sdpa
{
  namespace events
  {
    class SDPAEvent
    {
    public:
      typedef boost::shared_ptr<SDPAEvent> Ptr;

      virtual ~SDPAEvent() = default;

      virtual void handleBy
        (fhg::com::p2p::address_t const& source, EventHandler*) = 0;

    protected:
      SDPAEvent() = default;

    private:
      friend class boost::serialization::access;
      template <class Archive>
      void serialize (Archive &, unsigned int)
      {
      }
    };
  }
}

#include <sdpa/events/Serialization.hpp>

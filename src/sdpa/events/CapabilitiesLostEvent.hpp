#pragma once

#include <sdpa/events/MgmtEvent.hpp>
#include <sdpa/capability.hpp>

namespace sdpa
{
  namespace events
  {
    class CapabilitiesLostEvent : public MgmtEvent
    {
    public:
      typedef boost::shared_ptr<CapabilitiesLostEvent> Ptr;

      CapabilitiesLostEvent
        (const sdpa::capabilities_set_t& cpbs = capabilities_set_t())
          : MgmtEvent()
          , capabilities_ (cpbs)
      {}

      CapabilitiesLostEvent (const sdpa::capability_t &cap)
        : MgmtEvent()
        , capabilities_()
      {
        capabilities_.insert (cap);
      }

      const sdpa::capabilities_set_t& capabilities() const
      {
        return capabilities_;
      }

      virtual void handleBy
        (fhg::com::p2p::address_t const& source, EventHandler* handler) override
      {
        handler->handleCapabilitiesLostEvent (source, this);
      }

    private:
      sdpa::capabilities_set_t capabilities_;
    };

    SAVE_CONSTRUCT_DATA_DEF (CapabilitiesLostEvent, e)
    {
      SAVE_MGMTEVENT_CONSTRUCT_DATA (e);
      SAVE_TO_ARCHIVE (e->capabilities());
    }

    LOAD_CONSTRUCT_DATA_DEF (CapabilitiesLostEvent, e)
    {
      LOAD_MGMTEVENT_CONSTRUCT_DATA();
      LOAD_FROM_ARCHIVE (sdpa::capabilities_set_t, capabilities);

      ::new (e) CapabilitiesLostEvent (capabilities);
    }
  }
}

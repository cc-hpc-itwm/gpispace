#ifndef SDPA_CAPABILITIES_LOST_EVENT
#define SDPA_CAPABILITIES_LOST_EVENT 1

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
        ( const address_t& from
        , const address_t& to
        , const sdpa::capabilities_set_t& cpbs = capabilities_set_t()
        )
          : MgmtEvent (from, to)
          , capabilities_ (cpbs)
      {}

      CapabilitiesLostEvent ( const address_t& from
                            , const address_t& to
                            , const sdpa::capability_t &cap
                            )
        : MgmtEvent(from, to)
        , capabilities_()
      {
        capabilities_.insert (cap);
      }

      const sdpa::capabilities_set_t& capabilities() const
      {
        return capabilities_;
      }

      virtual void handleBy (EventHandler* handler) override
      {
        handler->handleCapabilitiesLostEvent (this);
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
      LOAD_MGMTEVENT_CONSTRUCT_DATA (from, to);
      LOAD_FROM_ARCHIVE (sdpa::capabilities_set_t, capabilities);

      ::new (e) CapabilitiesLostEvent (from, to, capabilities);
    }
  }
}

#endif

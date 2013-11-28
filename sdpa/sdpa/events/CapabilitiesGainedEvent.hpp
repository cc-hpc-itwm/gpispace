#ifndef SDPA_CAPABILITIES_GAINED_EVENT
#define SDPA_CAPABILITIES_GAINED_EVENT 1

#include <sdpa/events/MgmtEvent.hpp>
#include <sdpa/capability.hpp>

namespace sdpa
{
  namespace events
  {
    class CapabilitiesGainedEvent : public MgmtEvent
    {
    public:
      typedef boost::shared_ptr<CapabilitiesGainedEvent> Ptr;

      CapabilitiesGainedEvent
        ( const address_t& from
        , const address_t& to
        , const sdpa::capabilities_set_t& cpbs = capabilities_set_t()
        )
          : MgmtEvent (from, to)
          , capabilities_ (cpbs)
      {}

      CapabilitiesGainedEvent ( const address_t& from
                              , const address_t& to
                              , const sdpa::capability_t& cap
                              )
        : MgmtEvent (from, to)
        , capabilities_ ()
      {
        capabilities_.insert (cap);
      }

      const sdpa::capabilities_set_t& capabilities() const
      {
        return capabilities_;
      }

      std::string str() const
      {
        return "CapabilitiesGainedEvent";
      }

      virtual void handleBy (EventHandler* handler)
      {
        handler->handleCapabilitiesGainedEvent (this);
      }

    private:
      sdpa::capabilities_set_t capabilities_;
    };

    SAVE_CONSTRUCT_DATA_DEF (CapabilitiesGainedEvent, e)
    {
      SAVE_MGMTEVENT_CONSTRUCT_DATA (e);
      SAVE_TO_ARCHIVE (e->capabilities());
    }

    LOAD_CONSTRUCT_DATA_DEF (CapabilitiesGainedEvent, e)
    {
      LOAD_MGMTEVENT_CONSTRUCT_DATA (from, to);
      LOAD_FROM_ARCHIVE (sdpa::capabilities_set_t, capabilities);

      ::new (e) CapabilitiesGainedEvent (from, to, capabilities);
    }
  }
}

#endif

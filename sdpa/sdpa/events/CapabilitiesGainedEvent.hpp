#ifndef SDPA_CAPABILITIES_GAINED_EVENT
#define SDPA_CAPABILITIES_GAINED_EVENT 1

#include <sdpa/sdpa-config.hpp>

#include <sdpa/events/MgmtEvent.hpp>
#include <sdpa/capability.hpp>

namespace sdpa { namespace events {
	class CapabilitiesGainedEvent : public MgmtEvent {
	public:
		typedef sdpa::shared_ptr<CapabilitiesGainedEvent> Ptr;

        CapabilitiesGainedEvent() : MgmtEvent() {}

		CapabilitiesGainedEvent( const address_t& from, const address_t& to, const sdpa::capabilities_set_t& cpbs = capabilities_set_t() )
          : MgmtEvent(from, to)
          , capabilities_(cpbs)
        {}

          CapabilitiesGainedEvent( const address_t& from, const address_t& to, const sdpa::capability_t& cap )
          : MgmtEvent(from, to)
          , capabilities_()
          {
        	  capabilities_.insert (cap);
          }

          const sdpa::capabilities_set_t& capabilities() const { return capabilities_; }
          sdpa::capabilities_set_t& capabilities() { return capabilities_; }

          std::string str() const { return "CapabilitiesGainedEvent"; }

          virtual void handleBy(EventHandler *handler)
          {
        	  handler->handleCapabilitiesGainedEvent(this);
          }
	private:
          sdpa::capabilities_set_t capabilities_;
	};
}}

#endif

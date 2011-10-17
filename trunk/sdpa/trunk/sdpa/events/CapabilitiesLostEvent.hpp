/*
 * =====================================================================================
 *
 *       Filename:  CapabilitiesLostEvent.hpp
 *
 *    Description:  CapabilitiesLostEvent
 *
 *        Version:  1.0
 *        Created:
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Dr. Tiberiu Rotaru, tiberiu.rotaru@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */
#ifndef SDPA_CAPABILITIES_LOST_EVENT
#define SDPA_CAPABILITIES_LOST_EVENT 1

#include <sdpa/sdpa-config.hpp>

#ifdef USE_BOOST_SC
#   include <boost/statechart/event.hpp>
namespace sc = boost::statechart;
#endif
#include <sdpa/events/MgmtEvent.hpp>
#include <sdpa/types.hpp>

namespace sdpa { namespace events {
#ifdef USE_BOOST_SC
	class CapabilitiesLostEvent : public MgmtEvent, public sc::event<CapabilitiesLostEvent> {
#else
	class CapabilitiesLostEvent : public MgmtEvent {
#endif
	public:
		typedef sdpa::shared_ptr<CapabilitiesLostEvent> Ptr;

        CapabilitiesLostEvent() : MgmtEvent(){}

		CapabilitiesLostEvent( const address_t& from , const address_t& to, const sdpa::capabilities_set_t& cpbs = capabilities_set_t() )
          : MgmtEvent(from, to)
          , capabilities_(cpbs)
        {}

          CapabilitiesLostEvent( const address_t& from , const address_t& to, const sdpa::capability_t &cap)
            : MgmtEvent(from, to)
            , capabilities_()
          {
            capabilities_.insert (cap);
          }

		virtual ~CapabilitiesLostEvent() { }

		const sdpa::capabilities_set_t& capabilities() const { return capabilities_; }
		sdpa::capabilities_set_t& capabilities() { return capabilities_; }

		std::string str() const { return "CapabilitiesLostEvent"; }

        virtual void handleBy(EventHandler *handler)
        {
          handler->handleCapabilitiesLostEvent(this);
        }
	private:
		sdpa::capabilities_set_t capabilities_;
	};
}}

#endif

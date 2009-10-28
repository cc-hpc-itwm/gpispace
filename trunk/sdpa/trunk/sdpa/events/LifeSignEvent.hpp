#ifndef SDPA_LIFESIGNEVENT_HPP
#define SDPA_LIFESIGNEVENT_HPP 1

#include <sdpa/sdpa-config.hpp>

#ifdef USE_BOOST_SC
#   include <boost/statechart/event.hpp>
namespace sc = boost::statechart;
#endif
#include <sdpa/events/MgmtEvent.hpp>

namespace sdpa { namespace events {
#ifdef USE_BOOST_SC
	class LifeSignEvent : public MgmtEvent, public sc::event<LifeSignEvent> {
#else
	class LifeSignEvent : public MgmtEvent
#endif
	public:
		typedef sdpa::shared_ptr<LifeSignEvent> Ptr;

		LifeSignEvent(const address_t& from, const address_t& to ) :  sdpa::events::MgmtEvent(from, to) {}

		virtual ~LifeSignEvent() { }

		const sdpa::job_id_t & last_job_id() const { return last_job_id_; }

		std::string str() const { return "LifeSignEvent"; }
	private:
		sdpa::job_id_t last_job_id_;
	};
}}

#endif

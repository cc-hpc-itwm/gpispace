/*
 * =====================================================================================
 *
 *       Filename:  LifeSignEvent.hpp
 *
 *    Description:  LifeSignEvent
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
#ifndef SDPA_LIFESIGNEVENT_HPP
#define SDPA_LIFESIGNEVENT_HPP 1

#include <sdpa/sdpa-config.hpp>

#ifdef USE_BOOST_SC
#   include <boost/statechart/event.hpp>
namespace sc = boost::statechart;
#endif
#include <sdpa/events/MgmtEvent.hpp>
#include <sdpa/types.hpp>

namespace sdpa { namespace events {
#ifdef USE_BOOST_SC
	class LifeSignEvent : public MgmtEvent, public sc::event<LifeSignEvent> {
#else
	class LifeSignEvent : public MgmtEvent {
#endif
	public:
		typedef sdpa::shared_ptr<LifeSignEvent> Ptr;

        LifeSignEvent()
          : MgmtEvent()
          , last_job_id_("")
        {}

		LifeSignEvent(const address_t& from
                    , const address_t& to
                    , const sdpa::job_id_t &the_last_job = "")
          : MgmtEvent(from, to)
          , last_job_id_(the_last_job)
        {}

		virtual ~LifeSignEvent() { }

		const sdpa::job_id_t & last_job_id() const { return last_job_id_; }
		sdpa::job_id_t & last_job_id() { return last_job_id_; }

		std::string str() const { return "LifeSignEvent"; }

        virtual void handleBy(EventHandler *handler)
        {
          handler->handleLifeSignEvent(this);
        }
	private:
		sdpa::job_id_t last_job_id_;
	};
}}

#endif

/*
 * =====================================================================================
 *
 *       Filename:  Orchestrator.hpp
 *
 *    Description:  Contains the Orchestrator class
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
#ifndef SDPA_ORCHESTRATORTOR_HPP
#define SDPA_ORCHESTRATORTOR_HPP 1

#include <sdpa/daemon/daemonFSM/DaemonFSM.hpp>
#include <sdpa/daemon/orchestrator/SchedulerOrch.hpp>

namespace sdpa {
namespace daemon {

	class Orchestrator : public dsm::DaemonFSM
	{
	  public:
		typedef sdpa::shared_ptr<Orchestrator> ptr_t;
		SDPA_DECLARE_LOGGER();

		Orchestrator( const std::string &name = ""
		              , const std::string& url = ""
		              , unsigned int cap = 10000 )
		: DaemonFSM( name, sdpa::master_list_t(), cap, NULL),
		  SDPA_INIT_LOGGER(name),
		  url_(url)
		{
		  SDPA_LOG_DEBUG("Orchestrator's constructor called ...");
		}

		virtual ~Orchestrator();

		void action_configure( const sdpa::events::StartUpEvent& );
		void action_config_ok( const sdpa::events::ConfigOkEvent& );

		void handleJobFinishedEvent( const sdpa::events::JobFinishedEvent* );
		void handleJobFailedEvent( const sdpa::events::JobFailedEvent* );

		void handleCancelJobEvent( const sdpa::events::CancelJobEvent* pEvt );
		void handleCancelJobAckEvent( const sdpa::events::CancelJobAckEvent* pEvt );

		void handleRetrieveJobResultsEvent( const sdpa::events::RetrieveJobResultsEvent* pEvt );

		const std::string url() const {return url_;}
		bool is_orchestrator() { return true; }

		template <class Archive>
		void serialize(Archive& ar, const unsigned int)
		{
		    ar & boost::serialization::base_object<DaemonFSM>(*this);
		    ar & url_; //boost::serialization::make_nvp("url_", url_);
		}

		virtual void backup( std::ostream& );
		virtual void recover( std::istream& );

		friend class boost::serialization::access;

	  private:
		Scheduler* create_scheduler()
		{
		    DLOG(TRACE, "creating orchestrator scheduler...");
		    return new SchedulerOrch(this, useRequestModel());
		}

		std::string url_;
	  };
	}
}

#include <sdpa/daemon/orchestrator/Orchestrator.cpp>

#endif //SDPA_ORCHESTRATORTOR_HPP

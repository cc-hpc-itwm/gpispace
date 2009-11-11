#include <sdpa/daemon/daemonFSM/DaemonFSM.hpp>
#include "SchedulerNRE.hpp"
#include <gwes/GWES.h>
#include <gwes/IActivity.h>

namespace sdpa {
  class NRE : public dsm::DaemonFSM {
  public:
	typedef sdpa::shared_ptr<NRE> ptr_t;
	SDPA_DECLARE_LOGGER();

	NRE(const std::string &name, std::string& strAnswer )
		: dsm::DaemonFSM( name,  new gwes::GWES() ),
		  SDPA_INIT_LOGGER(name)
		{
			SDPA_LOG_DEBUG("NRE constructor called ...");
			ptr_scheduler_ =  sdpa::daemon::Scheduler::ptr_t(new SchedulerNRE(ptr_Sdpa2Gwes_, this, strAnswer));
		}


	virtual ~NRE()
	{
		SDPA_LOG_DEBUG("NRE destructor called ...");
		daemon_stage_ = NULL;
	}

	static NRE::ptr_t create(const std::string& name, std::string& strAnswer )
	{
		 return NRE::ptr_t(new NRE(name, strAnswer ));
	}

	static void start(NRE::ptr_t ptrNRE, std::string nreUrl = "127.0.0.1:5002", std::string masterUrl = "127.0.0.1:5001" )
	{
		dsm::DaemonFSM::create_daemon_stage(ptrNRE);
		ptrNRE->configure_network( nreUrl, sdpa::daemon::AGGREGATOR, masterUrl);
		sdpa::util::Config::ptr_t ptrCfg = sdpa::util::Config::create();
		dsm::DaemonFSM::start(ptrNRE, ptrCfg);
	}

	static void shutdown(NRE::ptr_t ptrNRE)
	{
		ptrNRE->shutdown_network();
		ptrNRE->stop();

		delete ptrNRE->ptr_Sdpa2Gwes_;
		ptrNRE->ptr_Sdpa2Gwes_ = NULL;
	}

	gwes::activity_id_t submitActivity(gwes::activity_t &activity)
	{
		ostringstream os;
		gwes::activity_id_t actId = activity.getID();
		gwes::workflow_id_t wfId  = activity.getOwnerWorkflowID();

		try {
			SDPA_LOG_DEBUG("NRE GWES submitted new activity ...");

			// push the job into the executor thread's queue
			//gwes::activity_t::ptr_t pAct(&activity);

			SDPA_LOG_DEBUG("Notify NRE GWES that the activity was dispatched ...");

			ptr_Sdpa2Gwes_->activityDispatched( wfId, actId );
			ptr_scheduler_->schedule(activity);
		}
		catch(std::exception&)
		{
			SDPA_LOG_DEBUG("Cancel the activity!");
			// inform immediately GWES that the corresponding activity was cancelled
			gwes()->activityCanceled( wfId, actId );
		}

		return activity.getID();
	}

  };
}

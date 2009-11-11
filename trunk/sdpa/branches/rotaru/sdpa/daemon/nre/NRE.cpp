#include <sdpa/daemon/daemonFSM/DaemonFSM.hpp>
#include "SchedulerNRE.hpp"
#include <gwes/GWES.h>
#include <NRE.hpp>

using namespace std;
using namespace sdpa::daemon;

NRE :: NRE(const std::string &name, std::string& strAnswer )
		: dsm::DaemonFSM( name,  new gwes::GWES() ),
		  SDPA_INIT_LOGGER(name)
{
	SDPA_LOG_DEBUG("NRE constructor called ...");
	ptr_scheduler_ =  sdpa::daemon::Scheduler::ptr_t(new SchedulerNRE(this, strAnswer));
}

NRE :: ~NRE()
{
	SDPA_LOG_DEBUG("NRE destructor called ...");
	daemon_stage_ = NULL;
}

NRE::ptr_t NRE ::create(const std::string& name, std::string& strAnswer )
{
	 return NRE::ptr_t(new NRE(name, strAnswer ));
}

void NRE :: start(NRE::ptr_t ptrNRE, std::string nreUrl, std::string masterUrl )
{
	dsm::DaemonFSM::create_daemon_stage(ptrNRE);
	ptrNRE->configure_network( nreUrl, sdpa::daemon::AGGREGATOR, masterUrl);
	sdpa::util::Config::ptr_t ptrCfg = sdpa::util::Config::create();
	dsm::DaemonFSM::start(ptrNRE, ptrCfg);
}

void NRE ::shutdown(NRE::ptr_t ptrNRE)
{
	ptrNRE->shutdown_network();
	ptrNRE->stop();

	delete ptrNRE->ptr_Sdpa2Gwes_;
	ptrNRE->ptr_Sdpa2Gwes_ = NULL;
}

gwes::activity_id_t  NRE ::submitActivity(gwes::activity_t &activity)
{
	SDPA_LOG_DEBUG("NRE GWES submitted new activity ...");
	ostringstream os;
	gwes::activity_id_t actId = activity.getID();
	gwes::workflow_id_t wfId  = activity.getOwnerWorkflowID();

	try {

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

void NRE ::cancelActivity(const gwes::activity_id_t &activityId) throw (gwes::Gwes2Sdpa::NoSuchActivity)
{
	SDPA_LOG_DEBUG("GWES asked SDPA to cancel the activity "<<activityId<<" ...");
	job_id_t job_id(activityId);

	//find a way to kill an activity!!!

}

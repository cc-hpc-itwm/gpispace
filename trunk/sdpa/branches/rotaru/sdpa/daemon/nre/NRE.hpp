#include <sdpa/daemon/daemonFSM/DaemonFSM.hpp>
#include "SchedulerNRE.hpp"
#include <gwes/GWES.h>

using namespace std;
using namespace sdpa::daemon;

namespace sdpa {
  class NRE : public dsm::DaemonFSM {
  public:
	typedef sdpa::shared_ptr<NRE> ptr_t;
	SDPA_DECLARE_LOGGER();

	NRE(const std::string &name, std::string& strAnswer )
		: DaemonFSM( name,  new gwes::GWES() ),
		  SDPA_INIT_LOGGER(name)
		{
			SDPA_LOG_DEBUG("NRE constructor called ...");
			ptr_scheduler_ =  Scheduler::ptr_t(new SchedulerNRE(ptr_Sdpa2Gwes_, this, strAnswer));
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
  };
}

#include <sdpa/daemon/daemonFSM/DaemonFSM.hpp>
#include <gwes/GWES.h>

namespace sdpa {
  class Orchestrator : public dsm::DaemonFSM {
  public:
	typedef sdpa::shared_ptr<Orchestrator> ptr_t;
	SDPA_DECLARE_LOGGER();

	Orchestrator(  const std::string &name )
		: DaemonFSM( name, new gwes::GWES() ),
		  SDPA_INIT_LOGGER(name)
	{
		SDPA_LOG_DEBUG("Orchestrator constructor called ...");
	}

	virtual ~Orchestrator()
	{
		SDPA_LOG_DEBUG("Orchestrator destructor called ...");
		daemon_stage_ = NULL;
	}

	static Orchestrator::ptr_t create(const std::string& name )
	{
		 return Orchestrator::ptr_t(new Orchestrator(name));
	}

	static void start(Orchestrator::ptr_t ptrOrch, std::string orchUrl = "127.0.0.1:5000" )
	{
		dsm::DaemonFSM::create_daemon_stage(ptrOrch);
		ptrOrch->configure_network(orchUrl);
		sdpa::util::Config::ptr_t ptrCfg = sdpa::util::Config::create();
		dsm::DaemonFSM::start(ptrOrch, ptrCfg);
	}

	static void shutdown(Orchestrator::ptr_t ptrOrch)
	{
		ptrOrch->shutdown_network();
		ptrOrch->stop();

		delete ptrOrch->ptr_Sdpa2Gwes_;
		ptrOrch->ptr_Sdpa2Gwes_ = NULL;
	}
  };
}

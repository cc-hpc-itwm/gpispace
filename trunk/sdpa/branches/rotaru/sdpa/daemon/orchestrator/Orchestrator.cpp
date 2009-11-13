#include <sdpa/daemon/daemonFSM/DaemonFSM.hpp>
#include <gwes/GWES.h>
#include <Orchestrator.hpp>

using namespace std;
using namespace sdpa::daemon;
using namespace sdpa::events;

Orchestrator::Orchestrator(  const std::string &name )
	: DaemonFSM( name, new gwes::GWES() ),
	  SDPA_INIT_LOGGER(name)
{
	SDPA_LOG_DEBUG("Orchestrator constructor called ...");
}

Orchestrator::~Orchestrator()
{
	SDPA_LOG_DEBUG("Orchestrator destructor called ...");
	daemon_stage_ = NULL;
}

Orchestrator::ptr_t Orchestrator::create( const std::string& name )
{
	return Orchestrator::ptr_t(new Orchestrator(name));
}

void Orchestrator::start( Orchestrator::ptr_t ptrOrch, std::string orchUrl )
{
	dsm::DaemonFSM::create_daemon_stage(ptrOrch);
	ptrOrch->configure_network(orchUrl);
	sdpa::util::Config::ptr_t ptrCfg = sdpa::util::Config::create();
	dsm::DaemonFSM::start(ptrOrch, ptrCfg);
}

void Orchestrator::shutdown( Orchestrator::ptr_t ptrOrch )
{
	ptrOrch->shutdown_network();
	ptrOrch->stop();

	delete ptrOrch->ptr_Sdpa2Gwes_;
	ptrOrch->ptr_Sdpa2Gwes_ = NULL;
}


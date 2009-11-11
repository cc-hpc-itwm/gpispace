#include <sdpa/daemon/daemonFSM/DaemonFSM.hpp>
#include <gwes/GWES.h>
#include <Aggregator.hpp>

using namespace std;
using namespace sdpa::daemon;


Aggregator::Aggregator(const std::string &name )
	: DaemonFSM( name, new gwes::GWES() ),
	  SDPA_INIT_LOGGER(name)
{
	SDPA_LOG_DEBUG("Aggregator constructor called ...");
}

Aggregator::~Aggregator()
{
	SDPA_LOG_DEBUG("Aggregator destructor called ...");
	daemon_stage_ = NULL;
}

Aggregator::ptr_t Aggregator::create(const std::string& name )
{
	 return Aggregator::ptr_t(new Aggregator(name));
}

void Aggregator::start(Aggregator::ptr_t ptrAgg, std::string aggUrl, std::string masterUrl )
{
	dsm::DaemonFSM::create_daemon_stage(ptrAgg);
	ptrAgg->configure_network(aggUrl, sdpa::daemon::ORCHESTRATOR, masterUrl);
	sdpa::util::Config::ptr_t ptrCfg = sdpa::util::Config::create();
	dsm::DaemonFSM::start(ptrAgg, ptrCfg);
}

void Aggregator::shutdown(Aggregator::ptr_t ptrAgg)
{
	ptrAgg->shutdown_network();
	ptrAgg->stop();

	delete ptrAgg->ptr_Sdpa2Gwes_;
	ptrAgg->ptr_Sdpa2Gwes_ = NULL;
}


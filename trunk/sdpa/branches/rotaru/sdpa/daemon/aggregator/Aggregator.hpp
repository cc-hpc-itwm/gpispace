#include <sdpa/daemon/daemonFSM/DaemonFSM.hpp>
#include <gwes/GWES.h>

namespace sdpa {
	namespace daemon {
	  class Aggregator : public dsm::DaemonFSM {
	  public:
		typedef sdpa::shared_ptr<Aggregator> ptr_t;
		SDPA_DECLARE_LOGGER();

		Aggregator(const std::string &name )
			: DaemonFSM( name, new gwes::GWES() ),
			  SDPA_INIT_LOGGER(name)
		{
			SDPA_LOG_DEBUG("Aggregator constructor called ...");
		}

		virtual ~Aggregator()
		{
			SDPA_LOG_DEBUG("Aggregator destructor called ...");
			daemon_stage_ = NULL;
		}

		static Aggregator::ptr_t create(const std::string& name )
		{
			 return Aggregator::ptr_t(new Aggregator(name));
		}

		static void start(Aggregator::ptr_t ptrAgg, std::string aggUrl = "127.0.0.1:5001", std::string masterUrl = "127.0.0.1:5000"   )
		{
			dsm::DaemonFSM::create_daemon_stage(ptrAgg);
			ptrAgg->configure_network(aggUrl, sdpa::daemon::ORCHESTRATOR, masterUrl);
			sdpa::util::Config::ptr_t ptrCfg = sdpa::util::Config::create();
			dsm::DaemonFSM::start(ptrAgg, ptrCfg);
		}

		static void shutdown(Aggregator::ptr_t ptrAgg)
		{
			ptrAgg->shutdown_network();
			ptrAgg->stop();

			delete ptrAgg->ptr_Sdpa2Gwes_;
			ptrAgg->ptr_Sdpa2Gwes_ = NULL;
		}
	  };
	}
}

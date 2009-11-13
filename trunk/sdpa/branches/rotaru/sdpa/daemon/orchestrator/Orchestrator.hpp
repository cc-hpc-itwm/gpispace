#include <sdpa/daemon/daemonFSM/DaemonFSM.hpp>
#include <gwes/GWES.h>

namespace sdpa {
namespace daemon {
	  class Orchestrator : public dsm::DaemonFSM {
	  public:
		typedef sdpa::shared_ptr<Orchestrator> ptr_t;
		SDPA_DECLARE_LOGGER();

		Orchestrator(  const std::string &name );

		virtual ~Orchestrator();

		static Orchestrator::ptr_t create(const std::string& name );
		static void start(Orchestrator::ptr_t ptrOrch, std::string orchUrl = "127.0.0.1:5000" );
		static void shutdown(Orchestrator::ptr_t ptrOrch);

	  };
	}
}

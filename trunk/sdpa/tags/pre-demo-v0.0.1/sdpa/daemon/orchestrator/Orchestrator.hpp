#include <sdpa/daemon/daemonFSM/DaemonFSM.hpp>
#include <gwes/GWES.h>

namespace sdpa {
namespace daemon {
	  class Orchestrator : public dsm::DaemonFSM {
	  public:
		typedef sdpa::shared_ptr<Orchestrator> ptr_t;
		SDPA_DECLARE_LOGGER();

		Orchestrator( const std::string &name,  const std::string& url );

		virtual ~Orchestrator();

		static Orchestrator::ptr_t create( const std::string& name,  const std::string& url );

		static void start(Orchestrator::ptr_t ptrOrch);
		static void shutdown(Orchestrator::ptr_t ptrOrch);

		void action_configure( const sdpa::events::StartUpEvent& );
		void action_config_ok(const sdpa::events::ConfigOkEvent&);

		void handleJobFinishedEvent(const sdpa::events::JobFinishedEvent* );
		void handleJobFailedEvent(const sdpa::events::JobFailedEvent* );

		void handleCancelJobEvent(const sdpa::events::CancelJobEvent* pEvt );
		void handleCancelJobAckEvent(const sdpa::events::CancelJobAckEvent* pEvt);

		const std::string& url() const {return url_;}

	  private:
		const std::string url_;
	  };
	}
}

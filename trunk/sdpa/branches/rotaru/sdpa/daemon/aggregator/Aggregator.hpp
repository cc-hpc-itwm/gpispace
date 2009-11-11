#include <sdpa/daemon/daemonFSM/DaemonFSM.hpp>
#include <gwes/GWES.h>

namespace sdpa {
	namespace daemon {
		class Aggregator : public dsm::DaemonFSM {
			public:
			typedef sdpa::shared_ptr<Aggregator> ptr_t;
			SDPA_DECLARE_LOGGER();

			Aggregator(const std::string &name );
			virtual ~Aggregator();
			static Aggregator::ptr_t create(const std::string& name );

			static void start(Aggregator::ptr_t ptrAgg, std::string aggUrl="127.0.0.1:5001", std::string masterUrl="127.0.0.1:5000");
			static void shutdown(Aggregator::ptr_t ptrAgg);
		};
	}
}

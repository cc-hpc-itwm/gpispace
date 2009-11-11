#include <sdpa/daemon/daemonFSM/DaemonFSM.hpp>
#include <gwes/GWES.h>
#include <gwes/IActivity.h>

namespace sdpa {
	namespace daemon {
	  class NRE : public dsm::DaemonFSM {
	  public:
		typedef sdpa::shared_ptr<NRE> ptr_t;
		SDPA_DECLARE_LOGGER();

		NRE(const std::string &name, std::string& strAnswer );
		virtual ~NRE();

		static NRE::ptr_t create(const std::string& name, std::string& strAnswer );
		static void start(NRE::ptr_t ptrNRE, std::string nreUrl = "127.0.0.1:5002", std::string masterUrl = "127.0.0.1:5001" );
		static void shutdown(NRE::ptr_t ptrNRE);

		gwes::activity_id_t submitActivity(gwes::activity_t &activity);
		void cancelActivity(const gwes::activity_id_t &activityId) throw (gwes::Gwes2Sdpa::NoSuchActivity);
	  };
	}
}

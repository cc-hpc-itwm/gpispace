/*
 * =====================================================================================
 *
 *       Filename:  NRE.hpp
 *
 *    Description:  Contains the NRE class
 *
 *        Version:  1.0
 *        Created:
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Dr. Tiberiu Rotaru, tiberiu.rotaru@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#include <sdpa/daemon/daemonFSM/DaemonFSM.hpp>
#include <gwes/GWES.h>
#include <gwes/IActivity.h>

namespace sdpa {
	namespace daemon {
	  class NRE : public dsm::DaemonFSM {
	  public:
		typedef sdpa::shared_ptr<NRE> ptr_t;
		SDPA_DECLARE_LOGGER();

		NRE( const std::string& name, const std::string& url,
			 const std::string& masterName, const std::string& masterUrl,
			 const std::string& workerUrl );

		virtual ~NRE();

		static NRE::ptr_t create( const std::string& name, const std::string& url,
								  const std::string& masterName, const std::string& masterUrl,
								  const std::string& workerUrl  );

		static void start( NRE::ptr_t ptrNRE );
		static void shutdown(NRE::ptr_t ptrNRE);

		void action_configure( const sdpa::events::StartUpEvent& );
		void action_config_ok( const sdpa::events::ConfigOkEvent& );

		gwes::activity_id_t submitActivity(gwes::activity_t &activity);
		void cancelActivity(const gwes::activity_id_t &activityId) throw (gwes::Gwes2Sdpa::NoSuchActivity);

		void handleJobFinishedEvent(const sdpa::events::JobFinishedEvent* );
		void handleJobFailedEvent(const sdpa::events::JobFailedEvent* );

		const std::string& url() const {return url_;}
		const std::string& masterName() const { return masterName_; }
		const std::string& masterUrl() const { return masterUrl_; }

	  private:
		const std::string url_;
		const std::string masterName_;
		const std::string masterUrl_;
	  };
	}
}

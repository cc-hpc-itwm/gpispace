/*
 * =====================================================================================
 *
 *       Filename:  NRE.hpp
 *
 *    Description:  Contains the NRE class
 *
 *        Version:  1.0
 *        Created:  2009
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

#include <sdpa/daemon/Observable.hpp>
#include <sdpa/daemon/NotificationService.hpp>

typedef sdpa::daemon::NotificationService gui_service;


namespace sdpa {
	namespace daemon {
	  class NRE : public dsm::DaemonFSM,  public sdpa::daemon::Observable {
	  public:
		typedef sdpa::shared_ptr<NRE> ptr_t;
		SDPA_DECLARE_LOGGER();

		NRE( const std::string& name, const std::string& url,
			 const std::string& masterName, const std::string& masterUrl,
			 const std::string& workerUrl,
			 const std::string& guiUrl);

		virtual ~NRE();

		static NRE::ptr_t create( const std::string& name, const std::string& url,
								  const std::string& masterName, const std::string& masterUrl,
								  const std::string& workerUrl,  const std::string guiUrl="127.0.0.1:9000");

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

		void activityCreated(const gwes::activity_t& act);
		void activityStarted(const gwes::activity_t& act);
		void activityFinished(const gwes::activity_t& act);
		void activityFailed(const gwes::activity_t& act);
		void activityCancelled(const gwes::activity_t& act);

	  private:
		const std::string url_;
		const std::string masterName_;
		const std::string masterUrl_;
		gui_service m_guiServ;
	  };
	}
}

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

#ifndef SDPA_NRE_HPP
#define SDPA_NRE_HPP 1

#include <sdpa/daemon/daemonFSM/DaemonFSM.hpp>

#include <sdpa/daemon/Observable.hpp>
#include <sdpa/daemon/NotificationService.hpp>
#include <sdpa/daemon/nre/SchedulerNRE.hpp>
#include <sdpa/daemon/nre/NreWorkerClient.hpp>

#include <boost/pointer_cast.hpp>

#include <sys/wait.h>

using namespace std;


typedef sdpa::daemon::NotificationService gui_service;


namespace sdpa {
	namespace daemon {
	  template <typename U>
	  class NRE : public dsm::DaemonFSM,  public sdpa::daemon::Observable {
	  public:
		typedef sdpa::shared_ptr<NRE<U> > ptr_t;
		//typedef typename T::internal_id_type we_internal_id_t;

		SDPA_DECLARE_LOGGER();

		NRE( const std::string& name = ""
                   , const std::string& url = ""
                   , const std::string& masterName = ""
                   //, const std::string& masterUrl = ""
                   , const std::string& workerUrl = ""
                   , const std::string& guiUrl = ""
                   // TODO: fixme, this is ugly
                   , bool bLaunchNrePcd = false
                   , const std::string & fvmPCBinary = ""
                   , const std::vector<std::string> & fvmPCSearchPath = std::vector<std::string>()
                   , const std::vector<std::string> & fvmPCPreLoad = std::vector<std::string>() )
					: dsm::DaemonFSM( name, NULL )
                , SDPA_INIT_LOGGER(name)
                , url_(url)
                , masterName_(masterName)
                , workerUrl_(workerUrl)
                , m_guiServ("SDPA", guiUrl)
                , bLaunchNrePcd_(bLaunchNrePcd)
                , nre_pcd_binary_(fvmPCBinary)
                , nre_pcd_search_path_(fvmPCSearchPath)
                , nre_pcd_pre_load_(fvmPCPreLoad)
		{
			SDPA_LOG_DEBUG("NRE's constructor called ...");

			// attach gui observer
			SDPA_LOG_DEBUG("Attach GUI observer ...");
			attach_observer(&m_guiServ);
		}

		virtual ~NRE()
		{
			SDPA_LOG_DEBUG("NRE's destructor called ...");

			//daemon_stage_ = NULL;
			detach_observer( &m_guiServ );
		}

		static ptr_t create( const std::string& name
						   , const std::string& url
						   , const std::string& masterName
						   //, const std::string& masterUrl
						   , const std::string& workerUrl
						   , const std::string guiUrl = "127.0.0.1:9000"
						   // TODO: fixme, this is ugly
						   , bool bLaunchNrePcd = false
						   , const std::string & fvmPCBinary = ""
						   , const std::vector<std::string> & fvmPCSearchPath = std::vector<std::string>()
						   , const std::vector<std::string> & fvmPCPreLoad = std::vector<std::string>()
						   )
		{
			 return ptr_t(new NRE<U>(  name
									   , url
									   , masterName
									   //, masterUrl
									   , workerUrl
									   , guiUrl
									   , bLaunchNrePcd
									   , fvmPCBinary
									   , fvmPCSearchPath
									   , fvmPCPreLoad
									   )
                                     );
		}

		void action_configure( const sdpa::events::StartUpEvent& );
		void action_config_ok( const sdpa::events::ConfigOkEvent& );

		bool cancel(const id_type&, const reason_type & );

		void handleJobFinishedEvent(const sdpa::events::JobFinishedEvent* );
		void handleJobFailedEvent(const sdpa::events::JobFailedEvent* );
		void handleCancelJobEvent(const CancelJobEvent* );
		void handleCancelJobAckEvent(const CancelJobAckEvent* );

		void notifyActivityCreated( const id_type& id, const std::string& data );
		void notifyActivityStarted( const id_type& id, const std::string& data );
		void notifyActivityFinished( const id_type& id, const std::string& data );
		void notifyActivityFailed( const id_type& id, const std::string& data );
		void notifyActivityCancelled( const id_type& id, const std::string& data );

		const std::string url() const {return url_;}
		const std::string masterName() const { return masterName_; }
		//const std::string& masterUrl() const { return masterUrl_; }

		template <class Archive>
		void serialize(Archive& ar, const unsigned int)
		{
			ar & boost::serialization::base_object<DaemonFSM>(*this);
			ar & url_; //boost::serialization::make_nvp("url_", url_);
			ar & masterName_; //boost::serialization::make_nvp("url_", masterName_);
			//ar & masterUrl_; //boost::serialization::make_nvp("url_", masterUrl_);
			ar & workerUrl_;
			//ar & m_guiServ;
		}

		virtual void backup( const bfs::path& strArchiveName );
	    virtual void recover( const bfs::path& strArchiveName );

		friend class boost::serialization::access;
		//friend class sdpa::tests::WorkerSerializationTest;

	  protected:

		Scheduler* create_scheduler()
		{
                  DLOG(TRACE, "creating nre scheduler...");
                  return new SchedulerNRE<U>( this
                                            , workerUrl_
                                            , bLaunchNrePcd_
                                            , nre_pcd_binary_
                                            , nre_pcd_search_path_
                                            , nre_pcd_pre_load_
                                            );
		}

		std::string url_;
		std::string masterName_;
		//std::string masterUrl_;
		std::string workerUrl_;
		gui_service m_guiServ;
		bool bLaunchNrePcd_;
        std::string nre_pcd_binary_;
        std::vector<std::string> nre_pcd_search_path_;
        std::vector<std::string> nre_pcd_pre_load_;
	  };
	}
}

// Implementation
#include <sdpa/daemon/nre/NRE.cpp>

#endif //SDPA_NRE_HPP

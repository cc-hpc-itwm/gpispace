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
#include <boost/pointer_cast.hpp>

namespace sdpa {
  namespace daemon {
      template <typename U>
      class NRE : public dsm::DaemonFSM,  public sdpa::daemon::Observable {
      public:
            typedef sdpa::shared_ptr<NRE<U> > ptr_t;

            SDPA_DECLARE_LOGGER();

            NRE( const std::string& name = ""
                 , const std::string& url = ""
                 , const sdpa::master_info_list_t arrMasterNames = sdpa::master_info_list_t()
                 , unsigned int cap = 10000
                 //, const std::string& masterUrl = ""
                 , const std::string& workerUrl = ""
                 , const std::string& guiUrl = ""
                 // TODO: fixme, this is ugly
                 , bool bLaunchNrePcd = false
                 , const std::string & fvmPCBinary = ""
                 , const std::vector<std::string> & fvmPCSearchPath = std::vector<std::string>()
                 , const std::vector<std::string> & fvmPCPreLoad = std::vector<std::string>()
               )

            : dsm::DaemonFSM( name, arrMasterNames, cap, NULL )
              , SDPA_INIT_LOGGER(name)
              , url_(url)
              , workerUrl_(workerUrl)
              , m_guiService("SDPA", guiUrl)
              , bLaunchNrePcd_(bLaunchNrePcd)
              , nre_pcd_binary_(fvmPCBinary)
              , nre_pcd_search_path_(fvmPCSearchPath)
              , nre_pcd_pre_load_(fvmPCPreLoad)
            {
              SDPA_LOG_DEBUG("NRE's constructor called ...");

              try
              {
                  if(!guiUrl.empty())
                  {
                    // logging gui service
                    m_guiService.open ();
                    // attach gui observer
                    SDPA_LOG_INFO("Logging GUI observer at " << guiUrl << " attached...");
                    attach_observer(&m_guiService);
                  }
              }
              catch (std::exception const & ex)
              {
                  SDPA_LOG_ERROR ("GUI observer at " << guiUrl << " could not be attached: " << ex.what());
              }
            }

            virtual ~NRE()
            {
              SDPA_LOG_DEBUG("NRE's destructor called ...");

              //daemon_stage_ = NULL;
              detach_observer( &m_guiService );
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

            virtual void feed_workers()
            {
              // eventually feed the process container daemons here!!!!
            }

            template <class Archive>
            void serialize(Archive& ar, const unsigned int)
            {
              ar & boost::serialization::base_object<DaemonFSM>(*this);
              ar & url_; //boost::serialization::make_nvp("url_", url_);
              ar & workerUrl_;
              ar & m_arrMasterInfo;
              //ar & m_guiService;
            }

            void cancelNotRunning(sdpa::job_id_t const & job);

            virtual void backup( std::ostream& );
            virtual void recover( std::istream& );

            friend class boost::serialization::access;

      protected:

            Scheduler* create_scheduler(bool bUseReqModel)
            {
              DLOG(TRACE, "creating nre scheduler...");

              return new SchedulerNRE<U>( this
                                          , workerUrl_
                                          , bLaunchNrePcd_
                                          , nre_pcd_binary_
                                          , nre_pcd_search_path_
                                          , nre_pcd_pre_load_
                                          , bUseReqModel
                                        );
            }

            std::string url_;
            //std::string masterName_;
            //std::string masterUrl_;
            std::string workerUrl_;
            NotificationService m_guiService;
            bool bLaunchNrePcd_;

            std::string nre_pcd_binary_;
            std::vector<std::string> nre_pcd_search_path_;
            std::vector<std::string> nre_pcd_pre_load_;
      };
}}

// Implementation
#include <sdpa/daemon/nre/NRE.cpp>

#endif //SDPA_NRE_HPP

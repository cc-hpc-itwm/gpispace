/*
 * =====================================================================================
 *
 *       Filename:  Aggreagtor.hpp
 *
 *    Description:  Contains the Agent class
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
#ifndef SDPA_AGENT_HPP
#define SDPA_AGENT_HPP 1

#include <sdpa/daemon/daemonFSM/DaemonFSM.hpp>
#include <sdpa/daemon/agent/AgentScheduler.hpp>
#include <sdpa/daemon/NotificationService.hpp>

namespace sdpa {
  namespace daemon {
    class Agent : public dsm::DaemonFSM
    {
      public:
        typedef sdpa::shared_ptr<Agent > ptr_t;
        SDPA_DECLARE_LOGGER();

        Agent(const std::string& name = "",
              const std::string& url = "",
              const sdpa::master_list_t arrMasterNames = sdpa::master_list_t(),
              unsigned int cap = 10000,
              const std::string& guiUrl = "")
        : DaemonFSM( name, arrMasterNames, cap, NULL ),
          SDPA_INIT_LOGGER(name),
          url_(url),
          m_guiService("SDPA", guiUrl)
        {
          /*BOOST_FOREACH(std::string master, arrMasterNames)
          {
            m_arrMasterNames.push_back(master);
          }*/

          SDPA_LOG_DEBUG("Agent's constructor called ...");
          //ptr_scheduler_ =  sdpa::daemon::Scheduler::ptr_t(new sdpa::daemon::Scheduler(this));

          // application gui service
          if(!guiUrl.empty())
          {
              m_guiService.open ();
              // attach gui observer
              SDPA_LOG_INFO("Application GUI service at " << guiUrl << " attached...");
          }
        }

        virtual ~Agent();

        void action_configure( const sdpa::events::StartUpEvent& );
        void action_config_ok( const sdpa::events::ConfigOkEvent& );

        void handleJobFinishedEvent(const sdpa::events::JobFinishedEvent* );
        void handleJobFailedEvent(const sdpa::events::JobFailedEvent* );

        void handleCancelJobEvent(const sdpa::events::CancelJobEvent* pEvt );
        void handleCancelJobAckEvent(const sdpa::events::CancelJobAckEvent* pEvt);

        void cancelNotRunning (sdpa::job_id_t const & job);

        const std::string url() const {return url_;}

        template <class Archive>
        void serialize(Archive& ar, const unsigned int)
        {
          ar & boost::serialization::base_object<DaemonFSM>(*this);
          ar & url_; //boost::serialization::make_nvp("url_", url_);
          //ar & m_arrMasterNames; //boost::serialization::make_nvp("url_", m_arrMasterNames);
        }

        virtual void backup( std::ostream& );
        virtual void recover( std::istream& );

        friend class boost::serialization::access;
        //friend class sdpa::tests::WorkerSerializationTest;

        void notifyAppGui(const result_type & result);

        private:
        Scheduler* create_scheduler()
        {
          DLOG(TRACE, "creating aggregator scheduler...");
          return new AgentScheduler(this);
        }

        std::string url_;

        NotificationService m_guiService;
      };
  }
}

#include <sdpa/daemon/agent/Agent.cpp>

#endif

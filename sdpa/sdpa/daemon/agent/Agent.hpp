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

#include <sdpa/daemon/DaemonFSM.hpp>
#include <sdpa/daemon/agent/AgentScheduler.hpp>
#include <sdpa/daemon/NotificationService.hpp>

namespace sdpa {
  namespace daemon {

    template <typename T> struct AgentFactory;

    class Agent : public sdpa::fsm::bmsm::DaemonFSM
    {
      public:
        typedef sdpa::shared_ptr<Agent > ptr_t;
        SDPA_DECLARE_LOGGER();

        Agent(const std::string& name = "",
              const std::string& url = "",
              const sdpa::master_info_list_t arrMasterNames = sdpa::master_info_list_t(),
              unsigned int cap = 10000,
              bool bCanRunTasksLocally = false,
              int rank = -1,
              const std::string& guiUrl = "")
        : DaemonFSM( name, arrMasterNames, cap, rank ),
          SDPA_INIT_LOGGER(name),
          url_(url),
          m_guiService("SDPA", guiUrl),
          m_bCanRunTasksLocally(bCanRunTasksLocally)
        {
          SDPA_LOG_DEBUG("Agent's constructor called ...");

          if(rank>=0)
          {
            std::ostringstream oss;
            oss<<"rank"<<rank;

            sdpa::capability_t properCpb(oss.str(), "rank", name);
            addCapability(properCpb);
          }

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

        void cancelPendingJob (const sdpa::events::CancelJobEvent& evt);

        bool finished(const id_type & id, const result_type & result);
        bool finished(const id_type & id, const result_type & result, const id_type& );
        bool failed( const id_type& workflowId, const result_type& result, int error_code, std::string const& reason);

        const std::string url() const {return url_;}

        template <class Archive>
        void serialize(Archive& ar, const unsigned int)
        {
          ar & boost::serialization::base_object<DaemonFSM>(*this);
          ar & url_; //boost::serialization::make_nvp("url_", url_);
          ar & m_bCanRunTasksLocally;
        }

        bool canRunTasksLocally() { return m_bCanRunTasksLocally; }
        virtual void backup( std::ostream& );
        virtual void recover( std::istream& );

        friend class boost::serialization::access;
        template <typename T> friend struct AgentFactory;

        template <typename T>
        void notifySubscribers(const T& ptrEvt);

        private:
        void createScheduler(bool bUseReqModel)
        {
          DLOG(TRACE, "creating agent scheduler...");
          Scheduler::ptr_t ptrSched( new AgentScheduler(this, bUseReqModel) );
          ptr_scheduler_ = ptrSched;
        }

        std::string url_;

        NotificationService m_guiService;
        bool m_bCanRunTasksLocally;
      };
  }
}

#endif

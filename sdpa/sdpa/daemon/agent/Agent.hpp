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

#include <sdpa/daemon/GenericDaemon.hpp>
#include <sdpa/daemon/scheduler/CoallocationScheduler.hpp>

namespace sdpa {
  namespace daemon {

    class Agent : public GenericDaemon
    {
      public:
        typedef sdpa::shared_ptr<Agent > ptr_t;
        SDPA_DECLARE_LOGGER();

        static Agent::ptr_t create ( const std::string& name
                                   , const std::string& url
                                   , const sdpa::master_info_list_t& arrMasterNames
                                   , const unsigned int rank = 0
                                   , const boost::optional<std::string>& appGuiUrl = boost::none
                                   );
        static Agent::ptr_t create_with_start_called ( const std::string& name
                                                     , const std::string& url
                                                     , const sdpa::master_info_list_t& arrMasterNames
                                                     , const unsigned int rank = 0
                                                     , const boost::optional<std::string>& appGuiUrl = boost::none
                                                     );

        Agent ( const std::string& name
              , const std::string& url
              , const sdpa::master_info_list_t arrMasterNames
              , int rank
              , const boost::optional<std::string>& guiUrl
              )
          : GenericDaemon( name, arrMasterNames, rank, guiUrl),
          SDPA_INIT_LOGGER(name),
          url_(url)
        {
          if(rank>=0)
          {
            std::ostringstream oss;
            oss<<"rank"<<rank;

            sdpa::capability_t properCpb(oss.str(), "rank", name);
            addCapability(properCpb);
          }
        }

        void handleJobFinishedEvent(const sdpa::events::JobFinishedEvent* );
        void handleJobFailedEvent(const sdpa::events::JobFailedEvent* );

        void handleCancelJobEvent(const sdpa::events::CancelJobEvent* pEvt );
        void handleCancelJobAckEvent(const sdpa::events::CancelJobAckEvent* pEvt);

        void cancelPendingJob (const sdpa::events::CancelJobEvent& evt);

        bool finished(const id_type & id, const result_type & result);
        bool failed( const id_type& workflowId, const result_type& result, int error_code, std::string const& reason);

        const std::string url() const {return url_;}

        template <typename T>
        void notifySubscribers(const T& ptrEvt);

        void pause(const job_id_t& id );
        void resume(const job_id_t& id );

      private:
        virtual void createScheduler()
        {
          ptr_scheduler_ = Scheduler::ptr_t (new CoallocationScheduler (this));
        }

      private:
        std::string url_;
      };
  }
}

#endif

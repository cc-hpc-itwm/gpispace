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
        Agent ( const std::string& name
              , const std::string& url
              , const sdpa::master_info_list_t arrMasterNames
              , int rank
              , const boost::optional<std::string>& guiUrl
              );

      protected:
        virtual void handleJobFinishedEvent(const sdpa::events::JobFinishedEvent* );
        virtual void handleJobFailedEvent(const sdpa::events::JobFailedEvent* );

        virtual void handleCancelJobEvent(const sdpa::events::CancelJobEvent* pEvt );
        virtual void handleCancelJobAckEvent(const sdpa::events::CancelJobAckEvent* pEvt);
        virtual void handleDeleteJobEvent(const sdpa::events::DeleteJobEvent* )
          { throw std::runtime_error("Not implemented.The agent should not call handleDeleteJobEvent!"); }
        virtual void handleDiscoverJobStatesEvent (const sdpa::events::DiscoverJobStatesEvent *pEvt);
        virtual void handleDiscoverJobStatesReplyEvent (const sdpa::events::DiscoverJobStatesReplyEvent *pEvt);

        virtual void finished(const we::layer::id_type & id, const we::type::activity_t&);
        virtual void failed( const we::layer::id_type& workflowId, int error_code, std::string const& reason);

        template <typename T>
        void notifySubscribers(const T& ptrEvt);

      private:
        CoallocationScheduler::ptr_t scheduler() const
        {
          return boost::static_pointer_cast<CoallocationScheduler> (ptr_scheduler_);
        }
      };
  }
}

#endif

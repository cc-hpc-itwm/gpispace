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

namespace sdpa {
  namespace daemon {

    class Agent : public GenericDaemon
    {
      public:
        Agent ( const std::string& name
              , const std::string& url
              , std::string kvs_host
              , std::string kvs_port
              , const sdpa::master_info_list_t arrMasterNames
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
      };
  }
}

#endif

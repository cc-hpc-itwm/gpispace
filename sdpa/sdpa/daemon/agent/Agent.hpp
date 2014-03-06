// tiberiu.rotaru@itwm.fraunhofer.de

#ifndef SDPA_AGENT_HPP
#define SDPA_AGENT_HPP 1

#include <sdpa/daemon/GenericDaemon.hpp>

namespace sdpa
{
  namespace daemon
  {
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
      virtual void handleJobFinishedEvent (const sdpa::events::JobFinishedEvent*);
      virtual void handleJobFailedEvent (const sdpa::events::JobFailedEvent*);

      virtual void handleCancelJobEvent (const sdpa::events::CancelJobEvent*);
      virtual void handleCancelJobAckEvent (const sdpa::events::CancelJobAckEvent*);
      virtual void handleDeleteJobEvent (const sdpa::events::DeleteJobEvent*)
      {
        throw std::runtime_error("The agent should not call handleDeleteJobEvent!");
      }
    };
  }
}

#endif

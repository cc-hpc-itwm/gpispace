/*
 * =====================================================================================
 *
 *       Filename:  AgentFactory.hpp
 *
 *    Description:
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
#ifndef SDPA_AGENT_FACTORY_HPP
#define SDPA_AGENT_FACTORY_HPP 1

#include <sdpa/daemon/agent/Agent.hpp>
#include <seda/Stage.hpp>
#include <seda/StageRegistry.hpp>
#include <seda/EventPrioQueue.hpp>
#include <seda/EventQueue.hpp>
#include <typeinfo>
#include <vector>
#include <string>

namespace agent {
	const int MAX_Q_SIZE = 5000;
}

namespace sdpa {
namespace daemon {
  template <typename T>
  struct AgentFactory
  {
    static Agent::ptr_t create( const std::string& name,
                                const std::string& url,
                                const sdpa::master_info_list_t& arrMasterNames,
                                const unsigned int capacity,
                                bool  bCanRunTasksLocally = false,
                                const unsigned int rank = 0,
                                const std::string& appGuiUrl = "" )
    {
      Agent::ptr_t pAgent( new Agent( name, url, arrMasterNames, capacity, bCanRunTasksLocally, rank, appGuiUrl ) );
      pAgent->createWorkflowEngine<T>();

//      seda::IEventQueue::Ptr ptrEvtPrioQueue(new seda::EventPrioQueue("network.stage."+name+".queue", agent::MAX_Q_SIZE));
//      seda::Stage::Ptr daemon_stage( new seda::Stage(name, ptrEvtPrioQueue, pAgent, 1) );
      seda::Stage::Ptr daemon_stage (new seda::Stage( name
                                                    , pAgent
                                                    , 1
                                                    )
                                    );

      pAgent->setStage(daemon_stage);
      seda::StageRegistry::instance().insert(daemon_stage);

      return pAgent;
    }
  };

  template <>
  struct AgentFactory<void>
  {
    static Agent::ptr_t create( const std::string& name,
                                const std::string& url,
                                const sdpa::master_info_list_t& arrMasterNames,
                                const unsigned int capacity,
                                bool  bCanRunTasksLocally = false,
                                const unsigned int rank = 0,
                                const std::string& appGuiUrl = "" )
    {
      LOG( DEBUG, "Create Agent "<<name<<" with no workflow engine" );
      Agent::ptr_t pAgent( new Agent( name, url, arrMasterNames, capacity, bCanRunTasksLocally, rank, appGuiUrl ) );

//      seda::IEventQueue::Ptr ptrEvtPrioQueue(new seda::EventPrioQueue("network.stage."+name+".queue", agent::MAX_Q_SIZE));
//      seda::Stage::Ptr daemon_stage( new seda::Stage(name, ptrEvtPrioQueue, pAgent, 1) );

      seda::Stage::Ptr daemon_stage (new seda::Stage( name
                                                    , pAgent
                                                    , 1
                                                    )
                                    );

      pAgent->setStage(daemon_stage);
      seda::StageRegistry::instance().insert(daemon_stage);

      return pAgent;
    }
  };
}}


#endif //SDPA_AGENT_HPP

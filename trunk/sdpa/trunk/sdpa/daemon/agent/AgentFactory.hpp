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
#include <typeinfo>
#include <vector>
#include <string>


namespace sdpa {
namespace daemon {
  template <typename T>
  struct AgentFactory
  {
    static Agent::ptr_t create( const std::string& name,
                                const std::string& url,
                                const sdpa::master_info_list_t& arrMasterNames,
                                const unsigned int capacity,
                                const std::string& appGuiUrl = "" )
    {
      LOG( DEBUG, "Create agent \""<<name<<"\" with an workflow engine of type "<<typeid(T).name() );
      Agent::ptr_t pAgent( new Agent( name, url, arrMasterNames, capacity, appGuiUrl ) );
      pAgent->create_workflow_engine<T>();
      seda::Stage::Ptr daemon_stage( new seda::Stage(name, pAgent, 1) );
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
                                const std::string& appGuiUrl = "" )
    {
      LOG( DEBUG, "Create Agent "<<name<<" with no workflow engine" );
      Agent::ptr_t pAgent( new Agent( name, url, arrMasterNames, capacity, appGuiUrl ) );
      seda::Stage::Ptr daemon_stage( new seda::Stage(name, pAgent, 1) );
      pAgent->setStage(daemon_stage);
      seda::StageRegistry::instance().insert(daemon_stage);

      return pAgent;
    }
  };
}}


#endif //SDPA_AGENT_HPP

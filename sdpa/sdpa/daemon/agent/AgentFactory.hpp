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
  struct AgentFactory
  {
    static Agent::ptr_t create_with_start_called( const std::string& name,
                                                const std::string& url,
                                                const sdpa::master_info_list_t& arrMasterNames,
                                                const unsigned int rank = 0,
                                                const boost::optional<std::string>& appGuiUrl = boost::none )
    {
      return Agent::create_with_start_called (name, url, arrMasterNames, rank, appGuiUrl);
    }
  };
}}


#endif //SDPA_AGENT_HPP

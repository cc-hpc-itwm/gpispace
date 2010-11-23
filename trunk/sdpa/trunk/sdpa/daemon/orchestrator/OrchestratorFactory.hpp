/*
 * =====================================================================================
 *
 *       Filename:  Orchestrator.hpp
 *
 *    Description:  Contains the Orchestrator class
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
#ifndef SDPA_ORCHESTRATORTOR_FACTORY_HPP
#define SDPA_ORCHESTRATORTOR_FACTORY_HPP 1

#include <sdpa/daemon/orchestrator/Orchestrator.hpp>
#include <sdpa/daemon/EmptyWorkflowEngine.hpp>


namespace sdpa {
namespace daemon {

		template <typename T>
		struct OrchestratorFactory
		{
			static Orchestrator::ptr_t create( 	const std::string& name,
												const std::string& url,
												const std::string &workflow_directory )
			{



				Orchestrator::ptr_t pOrch( new Orchestrator( name, url, workflow_directory) );
				pOrch->create_workflow_engine<T>();
				return pOrch;
			}
		};

		template <>
		struct OrchestratorFactory<void>
		{
			static Orchestrator::ptr_t create( 	const std::string& name,
												const std::string& url,
												const std::string &workflow_directory )
			{
				Orchestrator::ptr_t pOrch( new Orchestrator( name, url, workflow_directory) );
				return pOrch;
			}
		};

		/*template <>
		struct OrchestratorFactory<EmptyWorkflowEngine>
		{
			static Orchestrator::ptr_t create( 	const std::string& name,
												const std::string& url,
												const std::string &workflow_directory )
			{



				Orchestrator::ptr_t pOrch( new Orchestrator( name, url, workflow_directory) );
				pOrch->create_workflow_engine( new EmptyWorkflowEngine(pOrch.get()) );
				return pOrch;
			}
		};*/

	}
}


#endif //SDPA_ORCHESTRATORTOR_HPP

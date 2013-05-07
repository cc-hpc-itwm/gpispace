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
#include <seda/Stage.hpp>
#include <seda/StageRegistry.hpp>
#include <seda/EventPrioQueue.hpp>
#include <seda/EventQueue.hpp>
#include <typeinfo>

namespace orchestrator {
	const int MAX_Q_SIZE = 5000;
}

namespace sdpa {
namespace daemon {

		template <typename T>
		struct OrchestratorFactory
		{
			static Orchestrator::ptr_t create( const std::string& name,
			                                   const std::string& url,
			                                   const unsigned int capacity )
			{
				LOG( DEBUG, "Create orchestrator \""<<name<<"\" with an workflow engine of type "<<typeid(T).name() );
				Orchestrator::ptr_t pOrch( new Orchestrator( name, url, capacity ) );
				pOrch->createWorkflowEngine<T>();

//				seda::IEventQueue::Ptr ptrEvtPrioQueue(new seda::EventPrioQueue("network.stage."+name+".queue", orchestrator::MAX_Q_SIZE));
//				seda::Stage::Ptr daemon_stage( new seda::Stage(name, ptrEvtPrioQueue, pOrch, 1) );

				seda::Stage::Ptr daemon_stage (new seda::Stage(name, pOrch, 1));

				pOrch->setStage(daemon_stage);
				seda::StageRegistry::instance().insert(daemon_stage);

				return pOrch;
			}
		};

		template <>
		struct OrchestratorFactory<void>
		{
			static Orchestrator::ptr_t create( const std::string& name,
											   const std::string& url,
											   const unsigned int capacity )
			{
                          DMLOG (TRACE, "Create Orchestrator "<<name<<" with no workflow engine" );
				Orchestrator::ptr_t pOrch( new Orchestrator( name, url, capacity ) );

//				seda::IEventQueue::Ptr ptrEvtPrioQueue(new seda::EventPrioQueue("network.stage."+name+".queue", orchestrator::MAX_Q_SIZE));
//				seda::Stage::Ptr daemon_stage( new seda::Stage(name, ptrEvtPrioQueue, pOrch, 1) );

				seda::Stage::Ptr daemon_stage (new seda::Stage(name, pOrch, 1));

				pOrch->setStage(daemon_stage);
				seda::StageRegistry::instance().insert(daemon_stage);
				return pOrch;
			}
		};

	}
}


#endif //SDPA_ORCHESTRATORTOR_HPP

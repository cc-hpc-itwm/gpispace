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
#include <typeinfo>


namespace sdpa {
namespace daemon {

		template <typename T>
		struct OrchestratorFactory
		{
			static Orchestrator::ptr_t create( const std::string& name,
			                                   const std::string& url )
			{


                            LOG( DEBUG, "Create orchestrator \""<<name<<"\" with an workflow engine of type "<<typeid(T).name() );
                            Orchestrator::ptr_t pOrch( new Orchestrator( name, url, MAX_CAPACITY ) );
                            pOrch->create_workflow_engine<T>();
                            seda::Stage::Ptr daemon_stage( new seda::Stage(name, pOrch, 1) );
                            pOrch->setStage(daemon_stage);
                            seda::StageRegistry::instance().insert(daemon_stage);
                            return pOrch;
			}
		};

		template <>
		struct OrchestratorFactory<void>
		{
                    static Orchestrator::ptr_t create( const std::string& name,
                                                       const std::string& url )
                    {
                            LOG( DEBUG, "Create Orchestrator "<<name<<" with no workflow engine" );
                            Orchestrator::ptr_t pOrch( new Orchestrator( name, url, MAX_CAPACITY ) );
                            seda::Stage::Ptr daemon_stage( new seda::Stage(name, pOrch, 1) );
                            pOrch->setStage(daemon_stage);
                            seda::StageRegistry::instance().insert(daemon_stage);
                            return pOrch;
                    }
		};

	}
}


#endif //SDPA_ORCHESTRATORTOR_HPP

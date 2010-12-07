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
#ifndef SDPA_AGGREGATOR_FACTORY_HPP
#define SDPA_AGGREGATOR_FACTORY_HPP 1

#include <sdpa/daemon/aggregator/Aggregator.hpp>
#include <sdpa/daemon/EmptyWorkflowEngine.hpp>

#include <seda/Stage.hpp>
#include <seda/StageRegistry.hpp>

namespace sdpa {
namespace daemon {

		template <typename T>
		struct AggregatorFactory
		{
			static Aggregator::ptr_t create( 	const std::string& name,
												const std::string& url,
												const std::string &workflow_directory )
			{



				Aggregator::ptr_t pAgg( new Aggregator( name, url, workflow_directory) );
				pAgg->create_workflow_engine<T>();
				seda::Stage::Ptr daemon_stage( new seda::Stage(name, pAgg, 1) );
				pAgg->setStage(daemon_stage.get());
				seda::StageRegistry::instance().insert(daemon_stage);
				return pAgg;
			}
		};

		template <>
		struct AggregatorFactory<void>
		{
			static Aggregator::ptr_t create( 	const std::string& name,
												const std::string& url,
												const std::string &workflow_directory )
			{
				Aggregator::ptr_t pAgg( new Aggregator( name, url, workflow_directory) );
				seda::Stage::Ptr daemon_stage( new seda::Stage(name, pAgg, 1) );
				pAgg->setStage(daemon_stage.get());
				seda::StageRegistry::instance().insert(daemon_stage);
				return pAgg;
			}
		};
	}
}


#endif //SDPA_AGGREGATOR_HPP

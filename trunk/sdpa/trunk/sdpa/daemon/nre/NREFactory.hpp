/*
 * =====================================================================================
 *
 *       Filename:  NRE.hpp
 *
 *    Description:  Contains the NRE class
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
#ifndef SDPA_NRE_FACTORY_HPP
#define SDPA_NRE_FACTORY_HPP 1

#include <sdpa/daemon/nre/NRE.hpp>
#include <seda/Stage.hpp>
#include <seda/StageRegistry.hpp>
#include <typeinfo>

namespace sdpa {
namespace daemon {

		template <typename T, typename U >
		struct NREFactory
		{
			static typename NRE<U>::ptr_t create(   const std::string& name
												   , const std::string& url
												   , const std::string& masterName
												   , const std::string& workerUrl
												   , const std::string& appGuiUrl = "127.0.0.1:9000"
												   , const std::string& logGuiUrl = "127.0.0.1:9001"
												   , bool bLaunchNrePcd = false
												   , const std::string & fvmPCBinary = ""
												   , const std::vector<std::string> & fvmPCSearchPath = std::vector<std::string>()
												   , const std::vector<std::string> & fvmPCPreLoad = std::vector<std::string>()
												   )
			{



				LOG( DEBUG, "Create NRE \""<<name<<"\" with an workflow engine of type "<<typeid(T).name() );
				typename NRE<U>::ptr_t pNRE( new NRE<U>( name
													   , url
													   , masterName
													   , workerUrl
													   , appGuiUrl
													   , logGuiUrl
													   , bLaunchNrePcd
													   , fvmPCBinary
													   , fvmPCSearchPath
													   , fvmPCPreLoad
													   ) );

				pNRE->template create_workflow_engine<T>();
				seda::Stage::Ptr daemon_stage( new seda::Stage(name, pNRE, 1) );
				pNRE->setStage(daemon_stage);
				seda::StageRegistry::instance().insert(daemon_stage);
				return pNRE;
			}
		};

		template <typename U>
		struct NREFactory<void, U>
		{
			static typename NRE<U>::ptr_t create(  const std::string& name
												   , const std::string& url
												   , const std::string& masterName
												   , const std::string& workerUrl
												   , const std::string& appGuiUrl = "127.0.0.1:9000"
												   , const std::string& logGuiUrl = "127.0.0.1:9001"
												   , bool bLaunchNrePcd = false
												   , const std::string & fvmPCBinary = ""
												   , const std::vector<std::string> & fvmPCSearchPath = std::vector<std::string>()
												   , const std::vector<std::string> & fvmPCPreLoad = std::vector<std::string>()
												   )
			{
				LOG( DEBUG, "Create NRE "<<name<<" with no workflow engine " );
				typename NRE<U>::ptr_t pNRE( new NRE<U>(   name
														   , url
														   , masterName
														   , workerUrl
														   , appGuiUrl
														   , logGuiUrl
														   , bLaunchNrePcd
														   , fvmPCBinary
														   , fvmPCSearchPath
														   , fvmPCPreLoad
														   )
                      );

				seda::Stage::Ptr daemon_stage( new seda::Stage(name, pNRE, 1) );
				pNRE->setStage(daemon_stage);
				seda::StageRegistry::instance().insert(daemon_stage);

				return pNRE;
			}
		};

	}
}


#endif //SDPA_NRE_HPP

/*
 * =====================================================================================
 *
 *       Filename:  test_Components.cpp
 *
 *    Description:  test all components, each with a real gwes, using a real user client
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
#ifndef I_WORKER_CLIENT_HPP
#define I_WORKER_CLIENT_HPP 1

#include <sdpa/engine/IWorkflowEngine.hpp>

namespace sdpa { namespace nre { namespace worker {

#ifndef TIMEXCEEDEDEXCPT_
#define TIMEXCEEDEDEXCPT_
	 struct WalltimeExceeded : public std::runtime_error
	 {
		   WalltimeExceeded()
			 : std::runtime_error("execution did not finish within the specified walltime")
		   {}
		   virtual ~WalltimeExceeded() throw () {}
	 };
#endif

	struct BasicWorkerClient
	{
		 BasicWorkerClient( const std::string &nre_worker_location = ""
		                   , bool bLaunchNrePcd = false
		                   , const std::string & fvmPCBinary = ""
		                   , const std::vector<std::string> & fvmPCSearchPath = std::vector<std::string>()
		                   , const std::vector<std::string> & fvmPCPreLoad = std::vector<std::string>()
		                   ) {}

		virtual ~BasicWorkerClient() {}

		virtual void set_ping_interval(unsigned long ){} //seconds
		virtual void set_ping_timeout(unsigned long ){} //seconds
		virtual void set_ping_trials(std::size_t ){} //seconds

		virtual unsigned int start() throw (std::exception) { LOG( INFO, "Start the test NreWorkerClient ..."); return 0;}
		virtual void stop() { LOG( INFO, "Stop the test NreWorkerClient ...");}
		//virtual void cancel() throw (std::exception){ throw std::runtime_error("not implemented"); }

		virtual execution_result_t execute(const encoded_type& in_activity, unsigned long walltime = 0) throw (WalltimeExceeded, std::exception)
		{
			LOG( INFO, "Execute the activity "<<in_activity);
			execution_result_t exec_res(std::make_pair(ACTIVITY_FINISHED, "empty result"));
			LOG( INFO, "Report activity finished ...");

			return exec_res;
		}
	};
}}}

#endif

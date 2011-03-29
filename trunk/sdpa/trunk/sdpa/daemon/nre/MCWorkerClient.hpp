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
#ifndef MC_WORKER_CLIENT_HPP
#define MC_WORKER_CLIENT_HPP 1

#include <sdpa/engine/IWorkflowEngine.hpp>
#include <string>
#include <iostream>
#include <stdio.h>

#include <unistd.h>

int READ = 0;
int WRITE = 1;

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

	struct MCWorkerClient
	{
		 MCWorkerClient( const std::string &nre_worker_location = ""
		                   , bool bLaunchNrePcd = false
		                   , const std::string & fvmPCBinary = ""
		                   , const std::vector<std::string> & fvmPCSearchPath = std::vector<std::string>()
		                   , const std::vector<std::string> & fvmPCPreLoad = std::vector<std::string>()
		                   ) {}

		virtual ~MCWorkerClient() {}

		virtual void set_ping_interval(unsigned long ){} //seconds
		virtual void set_ping_timeout(unsigned long ){} //seconds
		virtual void set_ping_trials(std::size_t ){} //seconds

		virtual unsigned int start() throw (std::exception) { LOG( INFO, "Start the test NreWorkerClient ..."); return 0;}
		virtual void stop() { LOG( INFO, "Stop the test NreWorkerClient ...");}
		//virtual void cancel() throw (std::exception){ throw std::runtime_error("not implemented"); }

		std::string decode( const std::string& strInput )
		{
			std::string strOutput;
			std::stringstream sstr(strInput);
			boost::archive::text_iarchive ar(sstr);
			ar >> strOutput;
			return strOutput;
		}

		std::string encode( const std::string& strInput )
		{
			std::stringstream sstr;
			boost::archive::text_oarchive ar(sstr);
			ar << strInput;
			return sstr.str();
		}

		virtual execution_result_t execute(const encoded_type& in_activity, unsigned long walltime = 0) throw (WalltimeExceeded, std::exception)
		{
			LOG( INFO, "Execute the activity "<<in_activity);

			// decode the in_activity -> config file
			//portfolio_data_t portfolio_data;

			// call the asian backend
			//portfolio_data.decode(in_activity);
			std::string strCfg = decode(in_activity);

			const char* ASIAN_INPUT = strCfg.c_str();

			int p2c[2];
			int c2p[2];
			pid_t pid;
			char line[1024];

			if ( (pipe(p2c) < 0) || (pipe(c2p) < 0) )
			{
				LOG( ERROR,  "PIPE ERROR");
				return execution_result_t(std::make_pair(ACTIVITY_FAILED, "PIPE ERROR"));
			}
			if ( (pid = fork()) < 0 )
			{
				LOG( ERROR,  "FORK ERROR");
				return execution_result_t(std::make_pair(ACTIVITY_FAILED, "FORK ERROR" ));
			}
			else  if (pid == 0)     // CHILD PROCESS
			{
				close(p2c[WRITE]);
				close(c2p[READ]);

				if (p2c[READ] != STDIN_FILENO)
				{
					if (dup2(p2c[READ], STDIN_FILENO) != STDIN_FILENO)
					{
						LOG( ERROR,  "dup2 error to stdin");
					}
					close(p2c[READ]);
				}

				if (c2p[WRITE] != STDOUT_FILENO)
				{
					if (dup2(c2p[WRITE], STDOUT_FILENO) != STDOUT_FILENO)
					{
						LOG( ERROR,  "dup2 error to stdout");
					}
					close(c2p[WRITE]);
				}

				LOG( INFO,  "Call the Asian backend");
				if ( execl("./Asian", "Asian", (char *)0) < 0 )
				{
					LOG( ERROR,  "system error");
					return execution_result_t(std::make_pair(ACTIVITY_FAILED,  "system error" ));
				}

			}
			else        // PARENT PROCESS
			{
				int rv;
				close(p2c[READ]);
				close(c2p[WRITE]);

				if ( write(p2c[WRITE], ASIAN_INPUT, strlen(ASIAN_INPUT) ) != strlen(ASIAN_INPUT) )
				{
					LOG( ERROR,  "WRITE ERROR TO PIPE");
				}

				close(p2c[WRITE]);

				if ( (rv = read(c2p[READ], line, 1024)) < 0 )
				{
					LOG( ERROR,  "READ ERROR FROM PIPE");
				}
				else if (rv == 0)
				{
					LOG( ERROR,  "Child Closed Pipe");
					return execution_result_t(std::make_pair(ACTIVITY_FAILED,  "Child Closed Pipe" ));
				}


				LOG( INFO, "OUTPUT of Asian backend is: "<<line);

			}


			sdpa::job_result_t result( encode(line) );

			/*
			 * std::ostringstream sstr;
			boost::archive::text_oarchive ar(sstr);
			ar << line ;
			sdpa::job_result_t result(sstr.str());
			*/

			execution_result_t exec_res(std::make_pair(ACTIVITY_FINISHED, result));

			LOG(INFO, "Report activity finished ...");

			return exec_res;
		}
	};
}}}

#endif

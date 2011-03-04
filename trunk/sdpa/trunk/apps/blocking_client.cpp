#include <iostream>
#include <sstream>
#include <string>
#include <boost/program_options.hpp>
#include <sdpa/sdpa-config.hpp>
#include <sdpa/logging.hpp>
#include <sdpa/util/Config.hpp>

#include <sdpa/client/ClientApi.hpp>
#include <seda/StageRegistry.hpp>
#include <seda/Strategy.hpp>

//using namespace sdpa::daemon;
using namespace sdpa;
using namespace std;
using namespace seda;
namespace po = boost::program_options;

const int NMAXTRIALS = 10;
int m_nITER = 1;
int m_sleep_interval(1000000) ;

string read_workflow(string strFileName)
{
	ifstream f(strFileName.c_str());
	ostringstream os;
	os.str("");

	if( f.is_open() )
	{
		char c;
		while (f.get(c)) os<<c;
		f.close();
	}else
		cout<<"Unable to open file " << strFileName << ", error: " <<strerror(errno);

	return os.str();
}

// use FHGLOG_level=MIN FHGLOG_color=off

int main(int argc, char** argv)
{
	string orch;
	string file;

	po::options_description desc("Allowed options");
	desc.add_options()
	   ("help", "Display this message. To see logging messages, use FHGLOG_level=MIN and FHGLOG_color=off")
	   ("orchestrator,o",  po::value<std::string>(&orch)->default_value(""), "The orchestrator's name")
	   ("file,f", po::value<std::string>(&file)->default_value("stresstest.pnet"), "Workflow file")
	   ;

	po::variables_map vm;
	po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
	po::notify(vm);

	if (vm.count("help"))
	{
		std::cerr << "usage: bsdpac [options] ...." << std::endl;
		std::cerr << desc << std::endl;
		return 0;
	}

	std::cout <<"Starting the user client ..."<<std::endl;

	FHGLOG_SETUP();

	std::string strWorkflow;
	if(argc>1)
		strWorkflow = read_workflow(file);
	else
	{
		printf("please specify the file containing the workflow!");
		return -1;
	}

	sdpa::client::config_t config = sdpa::client::ClientApi::config();

	std::vector<std::string> cav;
	std::ostringstream oss;
	oss<<"--orchestrator="<<orch;
	cav.push_back(oss.str());
	config.parse_command_line(cav);

	sdpa::client::ClientApi::ptr_t ptrCli = sdpa::client::ClientApi::create( config );
	ptrCli->configure_network( config );


	for( int k=0; k<m_nITER; k++ )
	{
		int nTrials = 0;
		sdpa::job_id_t job_id_user;

		try {

			LOG( DEBUG, "Submitting the following test workflow: \n"<<strWorkflow);
			job_id_user = ptrCli->submitJob(strWorkflow);
		}
		catch(const sdpa::client::ClientException& cliExc)
		{
			if(nTrials++ > NMAXTRIALS)
			{
				LOG( DEBUG, "The maximum number of job submission  trials was exceeded. Giving-up now!");

				ptrCli->shutdown_network();
				ptrCli.reset();
				return -1;
			}
		}

		LOG( DEBUG, "*****JOB #"<<k<<"******");

		std::string job_status = ptrCli->queryJob(job_id_user);
		LOG( DEBUG, "The status of the job "<<job_id_user<<" is "<<job_status);

		nTrials = 0;
		while( job_status.find("Finished") == std::string::npos &&
			   job_status.find("Failed") == std::string::npos &&
			   job_status.find("Cancelled") == std::string::npos)
		{
			try {
				job_status = ptrCli->queryJob(job_id_user);
				LOG( DEBUG, "The status of the job "<<job_id_user<<" is "<<job_status);
				boost::this_thread::sleep(boost::posix_time::microseconds(5*m_sleep_interval));
			}
			catch(const sdpa::client::ClientException& cliExc)
			{
				LOG( DEBUG, "Exception: "<<cliExc.what());
				if(nTrials++ > NMAXTRIALS)
				{
					LOG( DEBUG, "The maximum number of job submission  trials was exceeded. Giving-up now!");

					ptrCli->shutdown_network();
					ptrCli.reset();
					return -1;
				}

				boost::this_thread::sleep(boost::posix_time::microseconds(5*m_sleep_interval));
			}
		}

		nTrials = 0;

		try {
				LOG( DEBUG, "User: retrieve results of the job "<<job_id_user);
				ptrCli->retrieveResults(job_id_user);
				boost::this_thread::sleep(boost::posix_time::microseconds(5*m_sleep_interval));
		}
		catch(const sdpa::client::ClientException& cliExc)
		{
			if(nTrials++ > NMAXTRIALS)
			{
				LOG( DEBUG, "The maximum number of job submission  trials was exceeded. Giving-up now!");

				ptrCli->shutdown_network();
				ptrCli.reset();
				return -1;
			}

			boost::this_thread::sleep(boost::posix_time::microseconds(5*m_sleep_interval));
		}

		nTrials = 0;

		try {
			LOG( DEBUG, "User: delete the job "<<job_id_user);
			ptrCli->deleteJob(job_id_user);
			boost::this_thread::sleep(boost::posix_time::microseconds(5*m_sleep_interval));
		}
		catch(const sdpa::client::ClientException& cliExc)
		{
			if(nTrials++ > NMAXTRIALS)
			{
				LOG( DEBUG, "The maximum number of job submission  trials was exceeded. Giving-up now!");

				ptrCli->shutdown_network();
				ptrCli.reset();
				return -1;
			}

			boost::this_thread::sleep(boost::posix_time::microseconds(5*m_sleep_interval));
		}
	}

	ptrCli->shutdown_network();

	//seda::StageRegistry::instance().stopAll();
	//seda::StageRegistry::instance().clear();
}

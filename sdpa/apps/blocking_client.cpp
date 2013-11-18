#include <iostream>
#include <sstream>
#include <string>
#include <boost/program_options.hpp>
#include <sdpa/sdpa-config.hpp>
#include <sdpa/logging.hpp>

#include <sdpa/client/ClientApi.hpp>
#include <seda/StageRegistry.hpp>
#include <seda/Strategy.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/tokenizer.hpp>
#include <fhgcom/kvs/kvsc.hpp>

#include <sdpa/events/JobFinishedEvent.hpp>
#include <sdpa/events/JobFailedEvent.hpp>
#include <sdpa/events/CancelJobAckEvent.hpp>
#include <sdpa/events/ErrorEvent.hpp>

#include <fhg/assert.hpp>

namespace bfs = boost::filesystem;

//using namespace sdpa::daemon;
using namespace sdpa;
using namespace std;
using namespace seda;
namespace po = boost::program_options;

const int NMAXTRIALS = 10;
int mPollingInterval(3000000) ; //3 microseconds

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
		LOG(FATAL, "Unable to open the workflow file " << strFileName << ", error: " <<strerror(errno));

	return os.str();
}


int main(int argc, char** argv)
{
	string orch;
	std::string strFileName;

	po::options_description desc("Allowed options");
	desc.add_options()
	   ("help", "Display this message. To see logging messages, use FHGLOG_level=MIN and FHGLOG_color=off")
	   ("orchestrator,o",  po::value<std::string>(&orch)->default_value("orchestrator"), "The orchestrator's name")
	   ("file,f", po::value<std::string>(&strFileName)->default_value("stresstest.pnet"), "Workflow file name")
	   ("kvs_url,k",  po::value<string>(), "The kvs daemon's url")
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

	FHGLOG_SETUP();

	if( !vm.count("kvs_url") )
	{
		LOG(ERROR, "The url of the kvs daemon was not specified!");
		return -1;
	}
	else
	{
		boost::char_separator<char> sep(":");
		boost::tokenizer<boost::char_separator<char> > tok(vm["kvs_url"].as<std::string>(), sep);

		vector< string > vec;
		vec.assign(tok.begin(),tok.end());

		if( vec.size() != 2 )
		{
			LOG(ERROR, "Invalid kvs url.  Please specify it in the form <hostname (IP)>:<port>!");
			return -1;
		}
		else
		{
			LOG(INFO, "The kvs daemon is assumed to run at "<<vec[0]<<":"<<vec[1]);
			fhg::com::kvs::global::get_kvs_info().init( vec[0], vec[1], boost::posix_time::seconds(10), 3);
		}
	}

	LOG(INFO, "Starting the user client ...");

	bfs::path pathWorkflowFile(strFileName);

	boost::filesystem::file_status st = boost::filesystem::status(pathWorkflowFile);

	if(!bfs::exists(st))
	{
		LOG(FATAL, "The file containing the workflow, "<<strFileName<<", does not exist! Aborting now!");
	}

	std::string strWorkflow = read_workflow(strFileName);



	sdpa::client::config_t config = sdpa::client::ClientApi::config();

	std::vector<std::string> cav;
	std::ostringstream oss;
	oss<<"--orchestrator="<<orch;
	cav.push_back(oss.str());
	config.parse_command_line(cav);

	sdpa::client::ClientApi::ptr_t ptrCli = sdpa::client::ClientApi::create( config );
	ptrCli->configure_network( config );

	sdpa::job_id_t job_id_user;
	int nTrials = 0;
	try {

		LOG( DEBUG, "Submitting the workflow "<<strWorkflow);
		job_id_user = ptrCli->submitJob(strWorkflow);
		LOG( DEBUG, "Got the job id "<<job_id_user);
	}
	catch(const sdpa::client::ClientException& cliExc)
	{
		if(nTrials++ > NMAXTRIALS)
		{
			LOG( DEBUG, "The maximum number of job submission  trials was exceeded. Giving-up now!");

			ptrCli->shutdown_network();
			return -1;
		}
	}

	ptrCli->subscribe(job_id_user);

	LOG( DEBUG, "The client successfully subscribed for orchestrator notifications ...");
  sdpa::status::code job_status (sdpa::status::UNKNOWN);
	try
	{
    seda::IEvent::Ptr event (c->waitForNotification (100000));

    if ( sdpa::events::JobFinishedEvent* evt
       = dynamic_cast<sdpa::events::JobFinishedEvent*> (event.get())
       )
    {
      fhg_assert (evt->job_id() == id);
      job_status = sdpa::status::FINISHED;
    }
    else if ( sdpa::events::JobFailedEvent* evt
            = dynamic_cast<sdpa::events::JobFailedEvent*> (event.get())
            )
    {
      fhg_assert (evt->job_id() == id);
      job_status = sdpa::status::FAILED;
    }
    else if ( sdpa::events::CancelJobAckEvent* evt
            = dynamic_cast<sdpa::events::CancelJobAckEvent*> (event.get())
            )
    {
      fhg_assert (evt->job_id() == id);
      job_status = sdpa::status::CANCELED;
    }
    else if ( sdpa::events::ErrorEvent* evt
            = dynamic_cast<sdpa::events::ErrorEvent*>(event.get())
            )
    {
			LOG(ERROR, "error during subscription: reason := "
						+ err->reason()
						+ " code := "
						+ boost::lexical_cast<std::string>(err->error_code())
						);

			ptrCli->shutdown_network();
			return -1;
		}
		else
		{
			LOG(ERROR, "unexpected reply: " << (reply ? reply->str() : "null"));
			ptrCli->shutdown_network();
			return -1;
		}
	}
	catch (const sdpa::client::Timedout &)
	{
		LOG(ERROR, "Timeout expired!");
		ptrCli->shutdown_network();
		return -1;
	}

	LOG( DEBUG, "The status of the job "<<job_id_user<<" is "<<sdpa::status::show (job_status));

	nTrials = 0;
	if(!sdpa::status::is_terminal (job_status))
	{
		LOG(ERROR, "Unexpected status, leave now ...");
		ptrCli->shutdown_network();
		return -1;
	}

	nTrials = 0;

	// reset trials counter
	nTrials = 0;
	try {
		LOG( INFO, "Delete the user job "<<job_id_user);
		ptrCli->deleteJob(job_id_user);
		boost::this_thread::sleep(boost::posix_time::microseconds(mPollingInterval));
	}
	catch(const sdpa::client::ClientException& cliExc)
	{
		if(nTrials++ > NMAXTRIALS)
		{
			LOG( DEBUG, "The maximum number of job submission  trials was exceeded. Giving-up now!");

			ptrCli->shutdown_network();
			return -1;
		}

		boost::this_thread::sleep(boost::posix_time::microseconds(mPollingInterval));
	}

	ptrCli->shutdown_network();

	return 0;
}

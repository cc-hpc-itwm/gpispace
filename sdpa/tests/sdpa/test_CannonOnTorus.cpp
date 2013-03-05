///
 // =====================================================================================
 //
 //       Filename:  test_Components.cpp
 //
 //    Description:  test all components, each with a real gwes, using a real user client
 //
 //        Version:  1.0
 //        Created:
 //       Revision:  none
 //       Compiler:  gcc
 //
 //         Author:  Dr. Tiberiu Rotaru, tiberiu.rotaru@itwm.fraunhofer.de
 //        Company:  Fraunhofer ITWM
 //
 // =====================================================================================
 ///
#define BOOST_TEST_MODULE TestCannonOnTorus
#include "sdpa/daemon/JobFSM.hpp"
#include <boost/test/unit_test.hpp>

#include <iostream>

#include <fhgcom/kvs/kvsd.hpp>
#include <fhgcom/kvs/kvsc.hpp>
#include <fhgcom/io_service_pool.hpp>
#include <fhgcom/tcp_server.hpp>

#include <boost/thread.hpp>

#include "tests_config.hpp"

#include "sdpa/memory.hpp"
#include "sdpa/logging.hpp"
#include "sdpa/daemon/DaemonFSM.hpp"
#include <seda/Strategy.hpp>
#include <sdpa/client/ClientApi.hpp>

#include <sdpa/daemon/orchestrator/OrchestratorFactory.hpp>
#include <sdpa/daemon/agent/AgentFactory.hpp>
#include <seda/StageRegistry.hpp>

#include <boost/filesystem/path.hpp>

#include <sdpa/engine/DummyWorkflowEngine.hpp>
#include <sdpa/engine/EmptyWorkflowEngine.hpp>

#include <TorusWorkflowEngineOrch.hpp>
#include <TorusWorkflowEngineAgent.hpp>

#include <boost/thread.hpp>
#include <boost/shared_array.hpp>
#include <sdpa/types.hpp>

#include <stdio.h>

const int NMAXTRIALS=5;
const int MAX_CAP = 100;
const int NAGENTS = 1;

const int BUNCH_SIZE = 1;
const int g_nTorusDim = 3;

size_t TorusWorkflowEngineAgent::m_nTorusDim = g_nTorusDim;
size_t TorusWorkflowEngineOrch::m_nTorusDim = g_nTorusDim;

namespace po = boost::program_options;

using namespace std;
using namespace sdpa::tests;

#define NO_GUI ""

static const std::string kvs_host () { static std::string s("localhost"); return s; }
static const std::string kvs_port () { static std::string s("0"); return s; }

struct MyFixture
{
        MyFixture()
                        : m_nITER(1)
                        , m_sleep_interval(1000000)
                        , m_pool (0)
                , m_kvsd (0)
                , m_serv (0)
                , m_thrd (0)
                        , m_arrAgentMasterInfo(1, MasterInfo("orchestrator_0"))
        {
                //initialize and start_agent the finite state machine

                FHGLOG_SETUP();


                LOG(DEBUG, "Fixture's constructor called ...");

                m_pool = new fhg::com::io_service_pool(1);
                m_kvsd = new fhg::com::kvs::server::kvsd ("");
                m_serv = new fhg::com::tcp_server ( *m_pool
                                                                                  , *m_kvsd
                                                                                  , kvs_host ()
                                                                                  , kvs_port ()
                                                                                  , true
                                                                                  );
                m_thrd = new boost::thread (boost::bind ( &fhg::com::io_service_pool::run
                                                                                                , m_pool
                                                                                                )
                                                                   );

                m_serv->start();

                LOG(INFO, "kvs daemon is listening on port " << m_serv->port ());

                fhg::com::kvs::global::get_kvs_info().init( kvs_host()
                                                                                                  , boost::lexical_cast<std::string>(m_serv->port())
                                                                                                  , boost::posix_time::seconds(10)
                                                                                                  , 3
                                                                                                  );

                m_strWorkflow = read_workflow("workflows/stresstest.pnet");
        }

        ~MyFixture()
        {
                LOG(DEBUG, "Fixture's destructor called ...");

                sstrOrch.str("");
                sstrAgent.str("");

                m_serv->stop ();
                m_pool->stop ();
                m_thrd->join ();

                delete m_thrd;
                delete m_serv;
                delete m_kvsd;
                delete m_pool;

                seda::StageRegistry::instance().stopAll();
                seda::StageRegistry::instance().clear();
        }

        void run_cannon_client();

        int subscribe_and_wait ( const std::string &job_id, const sdpa::client::ClientApi::ptr_t &ptrCli );

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

        int m_nITER;
        int m_sleep_interval ;
    std::string m_strWorkflow;

    fhg::com::io_service_pool *m_pool;
        fhg::com::kvs::server::kvsd *m_kvsd;
        fhg::com::tcp_server *m_serv;
        boost::thread *m_thrd;

        sdpa::master_info_list_t m_arrAgentMasterInfo;

        std::stringstream sstrOrch;
        std::stringstream sstrAgent;

        boost::thread m_threadClient;
};

int MyFixture::subscribe_and_wait ( const std::string &job_id, const sdpa::client::ClientApi::ptr_t &ptrCli )
{
        typedef boost::posix_time::ptime time_type;
        time_type poll_start = boost::posix_time::microsec_clock::local_time();

        int exit_code(4);

        bool bSubscribed = false;

        do
        {
                try
                {
                        ptrCli->subscribe(job_id);
                        bSubscribed = true;
                }
                catch(...)
                {
                        bSubscribed = false;
                        boost::this_thread::sleep(boost::posix_time::seconds(1));
                }

        }while(!bSubscribed);

        if(bSubscribed)
                LOG(INFO, "The client successfully subscribed for orchestrator notifications ...");

        std::string job_status;

        int nTrials = 0;
        do {

                LOG(INFO, "start waiting at: " << poll_start);

                try
                {
                        if(nTrials<NMAXTRIALS)
                        {
                                boost::this_thread::sleep(boost::posix_time::seconds(3));
                                LOG(INFO, "Re-trying ...");

                                bSubscribed = false;

                                do
                                {
                                        try
                                        {
                                                ptrCli->subscribe(job_id);
                                                bSubscribed = true;
                                        }
                                        catch(...)
                                        {
                                                bSubscribed = false;
                                        }

                                }while(!bSubscribed);

                                if(bSubscribed)
                                        LOG(INFO, "The client successfully subscribed for orchestrator notifications ...");

                        }

                        seda::IEvent::Ptr reply( ptrCli->waitForNotification(10000) );

                        // check event type
                        if (dynamic_cast<sdpa::events::JobFinishedEvent*>(reply.get()))
                        {
                                job_status="Finished";
                                exit_code = 0;
                        }
                        else if (dynamic_cast<sdpa::events::JobFailedEvent*>(reply.get()))
                        {
                                job_status="Failed";
                                exit_code = 1;
                        }
                        else if (dynamic_cast<sdpa::events::CancelJobAckEvent*>(reply.get()))
                        {
                                job_status="Cancelled";
                                exit_code = 2;
                        }
                        else if(sdpa::events::ErrorEvent *err = dynamic_cast<sdpa::events::ErrorEvent*>(reply.get()))
                        {
                                std::cerr<< "got error event: reason := "
                                                        + err->reason()
                                                        + " code := "
                                                        + boost::lexical_cast<std::string>(err->error_code())<<std::endl;

                        }
                        else
                        {
                                LOG(WARN, "unexpected reply: " << (reply ? reply->str() : "null"));
                        }
                }
                catch (const sdpa::client::Timedout &)
                {
                        LOG(INFO, "Timeout expired!");
                }

        }while(exit_code == 4 && ++nTrials<NMAXTRIALS);

        std::cout<<"The status of the job "<<job_id<<" is "<<job_status<<std::endl;

        if( job_status != std::string("Finished") &&
                job_status != std::string("Failed")   &&
                job_status != std::string("Cancelled") )
        {
                LOG(ERROR, "Unexpected status, leave now ...");
                return exit_code;
        }

        time_type poll_end = boost::posix_time::microsec_clock::local_time();

        LOG(INFO, "Client stopped waiting at: " << poll_end);
        LOG(INFO, "Execution time: " << (poll_end - poll_start));
        return exit_code;
}

void MyFixture::run_cannon_client()
{
        sdpa::client::config_t config = sdpa::client::ClientApi::config();

        std::vector<std::string> cav;
        cav.push_back("--orchestrator=orchestrator_0");
        cav.push_back("--network.timeout=-1");
        config.parse_command_line(cav);

        std::ostringstream osstr;
        osstr<<"sdpac_"<<0;

        LOG(INFO, "Starting client_0 ...");
        int nBlockDim = 3;

        matrix_t A(g_nTorusDim*nBlockDim, g_nTorusDim*nBlockDim);
        matrix_t B(g_nTorusDim*nBlockDim, g_nTorusDim*nBlockDim);
        // initialize m

        for( int i=0; i<g_nTorusDim*nBlockDim; i++)
                for( int j=0; j<g_nTorusDim*nBlockDim; j++)
                {
                        A(i,j) = i+j;
                        B(i,j) = i+j+1;
                }

        std::stringstream sstr;
        boost::archive::text_oarchive ar(sstr);
        ar << (matrix_t const&)A;
        ar << (matrix_t const&)B;
        ar << g_nTorusDim;
        m_strWorkflow = sstr.str();

        LOG( DEBUG, "The test workflow is "<<m_strWorkflow);

        sdpa::client::ClientApi::ptr_t ptrCli = sdpa::client::ClientApi::create( config, osstr.str(), osstr.str()+".apps.client.out" );
        ptrCli->configure_network( config );

        int nTrials = 0;
        sdpa::job_id_t job_id_user;

        try {

                LOG( DEBUG, "Submitting new workflow ..."); //<<m_strWorkflow);
                job_id_user = ptrCli->submitJob(m_strWorkflow);
        }
        catch(const sdpa::client::ClientException& cliExc)
        {
                if(nTrials++ > NMAXTRIALS)
                {
                        LOG( DEBUG, "The maximum number of job submission  trials was exceeded. Giving-up now!");

                        ptrCli->shutdown_network();
                        ptrCli.reset();
                        return;
                }
        }


        int exit_code = subscribe_and_wait( job_id_user, ptrCli );

        try {
                LOG( DEBUG, "User: delete the job "<<job_id_user);
                ptrCli->deleteJob(job_id_user);
                boost::this_thread::sleep(boost::posix_time::seconds(3));
        }
        catch(const sdpa::client::ClientException& cliExc)
        {
                LOG( DEBUG, "The maximum number of  trials was exceeded. Giving-up now!");

                ptrCli->shutdown_network();
                ptrCli.reset();
                return;

                boost::this_thread::sleep(boost::posix_time::seconds(3));
        }

        ptrCli->shutdown_network();
        boost::this_thread::sleep(boost::posix_time::microseconds(5*m_sleep_interval));
    ptrCli.reset();
}

BOOST_FIXTURE_TEST_SUITE( test_agents, MyFixture )

BOOST_AUTO_TEST_CASE( testCannonParMM )
{
        LOG( DEBUG, "////////// testCannon'sAlg //////////");

        //guiUrl
        string addrOrch         = "127.0.0.1";
        string addrAgent        = "127.0.0.1";

        //LOG( DEBUG, "Create Agent with an Dummy workflow engine ...");
        sdpa::master_info_list_t list_masters;

        sdpa::master_info_list_t arrOrchMasterInfo;
        sdpa::daemon::Agent::ptr_t ptrOrch = sdpa::daemon::AgentFactory<TorusWorkflowEngineOrch>::create("orchestrator_0", addrOrch, arrOrchMasterInfo, MAX_CAP);
        ptrOrch->start_agent(false);

        //LOG( DEBUG, "Create the Aggregator ...");
        //sdpa::daemon::Agent::ptr_t arrAgents[NAGENTS];
        boost::shared_array<sdpa::daemon::Agent::ptr_t> arrAgents( new sdpa::daemon::Agent::ptr_t[g_nTorusDim*g_nTorusDim] );

        sdpa::master_info_list_t arrAgentMasterInfo(1, MasterInfo("orchestrator_0"));
        // all agents have a the orchestrator as master -> initial distribution
        for(int k=0; k<g_nTorusDim*g_nTorusDim; k++)
        {
                ostringstream oss;
                oss<<"agent_"<<k;

                arrAgents[k] = sdpa::daemon::AgentFactory<TorusWorkflowEngineAgent>::create( oss.str(),
                                                                                                                                                                         addrAgent,
                                                                                                                                                                         arrAgentMasterInfo,
                                                                                                                                                                         MAX_CAP,
                                                                                                                                                                         false,  // can execute jobs
                                                                                                                                                                         k);    // rank
        }

        for(int k=0; k<g_nTorusDim*g_nTorusDim; k++)
                arrAgents[k]->start_agent(false);

        boost::this_thread::sleep(boost::posix_time::seconds(5));
        LOG(INFO, "create agents communication topology");

        LOG(INFO, "On the horizontal axis:");
        for(int i=0; i<g_nTorusDim; i++)
                for(int j=0; j<g_nTorusDim; j++)
                {
                        int right = i*g_nTorusDim+(j+1)%g_nTorusDim;

                        ostringstream oss;
                        oss<<"agent_"<<i*g_nTorusDim+j;

                        arrAgents[right]->addMaster(oss.str());
                        LOG(INFO, "added new master of agent_"<<right<<" -> "<<oss.str());
                }


        LOG(INFO, "On the vertical axis:");
        for(int i=0; i<g_nTorusDim; i++)
                for(int j=0; j<g_nTorusDim; j++)
                {
                        int down = ((i+1)%g_nTorusDim)*g_nTorusDim+j;

                        ostringstream oss;
                        oss<<"agent_"<<i*g_nTorusDim+j;

                        arrAgents[down]->addMaster(oss.str());
                        LOG(INFO, "added new master of agent_"<<down<<" -> "<<oss.str());
                }

         boost::this_thread::sleep(boost::posix_time::seconds(10));

        for(int k=0; k<g_nTorusDim*g_nTorusDim; k++)
        {
                sdpa::capabilities_set_t agentCpbSet;
                arrAgents[k]->getCapabilities(agentCpbSet);
                std::cout<<"The capabilities of "<<arrAgents[k]->name()<< " are:"<<std::endl;
                std::cout<<agentCpbSet<<std::endl;
        }

        boost::thread threadClient = boost::thread(boost::bind(&MyFixture::run_cannon_client, this));
        threadClient.join();
        LOG( INFO, "The client thread joined the main thread°!" );

        LOG(INFO, "On the horizontal axis:");
                for(int i=0; i<g_nTorusDim; i++)
                        for(int j=0; j<g_nTorusDim; j++)
                        {
                                int right = i*g_nTorusDim+(j+1)%g_nTorusDim;

                                ostringstream oss;
                                oss<<"agent_"<<i*g_nTorusDim+j;

                                arrAgents[right]->removeMaster(oss.str());
                                LOG(INFO, "remove master of agent_"<<right<<" -> "<<oss.str());
                        }

        LOG(INFO, "On the vertical axis:");
        for(int i=0; i<g_nTorusDim; i++)
                for(int j=0; j<g_nTorusDim; j++)
                {
                        int down = ((i+1)%g_nTorusDim)*g_nTorusDim+j;

                        ostringstream oss;
                        oss<<"agent_"<<i*g_nTorusDim+j;

                        arrAgents[down]->removeMaster(oss.str());
                        LOG(INFO, "remove master of agent_"<<down<<" -> "<<oss.str());
                }

        for(int k=0; k<g_nTorusDim*g_nTorusDim; k++)
                arrAgents[k]->shutdown();
        ptrOrch->shutdown();

        LOG( DEBUG, "The test case testAgentNoWe terminated!");
}

BOOST_AUTO_TEST_SUITE_END()

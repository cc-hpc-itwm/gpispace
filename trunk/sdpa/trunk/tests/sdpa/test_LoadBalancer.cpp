/*
* =====================================================================================
*
*       Filename:  test_LoadBalancer.cpp
*
*    Description:  test the scheduler thread
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
#define BOOST_TEST_MODULE TestLoadBalancer

#include <sdpa/daemon/jobFSM/JobFSM.hpp>
#include <boost/test/unit_test.hpp>
#include <iostream>
#include <string>
#include <list>
#include <sdpa/memory.hpp>
#include <time.h>
#include <sdpa/util/util.hpp>
#include <fstream>
#include <sdpa/daemon/WorkerManager.hpp>
#include <sstream>
#include <sdpa/JobId.hpp>

using namespace std;
using namespace sdpa::tests;
using namespace sdpa::daemon;

const int NWORKERS = 10;

struct MyFixture
{
  MyFixture() :SDPA_INIT_LOGGER("sdpa.tests.testLoadBalancer"){}
  ~MyFixture()
  {
          seda::StageRegistry::instance().stopAll();
          seda::StageRegistry::instance().clear();
  }

  SDPA_DECLARE_LOGGER();
};

BOOST_FIXTURE_TEST_SUITE( test_LoadBalancer, MyFixture )

BOOST_AUTO_TEST_CASE(testLoadBalancer)
{
  WorkerManager wm;
  ostringstream oss;
  size_t arrLoad[] = {100, 200, 10, 2, 30, 1000, 5, 300, 55, 701};

  // create a number of workers
  for(int k=0;k<NWORKERS;k++)
  {
    oss.str("");
    oss<<"TestWorker_"<<k;
    sdpa::JobId id;
    wm.addWorker(oss.str(), k, id.str());
  }

  // submit jobs to the workers
  for(int k=0; k<NWORKERS; k++)
  {
    try
    {
      const Worker::ptr_t& ptrCurrWorker = wm.getNextWorker();

      for( size_t l=0; l<arrLoad[k]; l++)
      {
              sdpa::job_id_t jobId;
              ptrCurrWorker->dispatch(jobId);
      }
    }
    catch(const NoWorkerFoundException& ex)
    {
      SDPA_LOG_ERROR("No worker found!");
      BOOST_CHECK(false);
      break;
    }
  }

  // print loads before balancing
  for(int k=0; k<NWORKERS; k++)
  {
    try
    {
      const Worker::ptr_t& ptrCurrWorker = wm.getNextWorker();
      cout<<"load["<<ptrCurrWorker->name()<<"] = "<<ptrCurrWorker->pending().size()<<std::endl;
    }
    catch(const NoWorkerFoundException& ex)
    {
      SDPA_LOG_ERROR("No worker found!");
      BOOST_CHECK(false);
      break;
    }
  }

  cout<<"Balance the workers now ..."<<endl<<endl;
  wm.balanceWorkers();

  // print load after balancing
  for(int k=0; k<NWORKERS; k++)
  {
    try
    {
      const Worker::ptr_t& ptrCurrWorker = wm.getNextWorker();
      cout<<"load["<<ptrCurrWorker->name()<<"] = "<<ptrCurrWorker->pending().size()<<std::endl;
    }
    catch(const NoWorkerFoundException& ex)
    {
      SDPA_LOG_ERROR("No worker found!");
      BOOST_CHECK(false);
      break;
    }
  }
}

BOOST_AUTO_TEST_SUITE_END()

/*
 * =====================================================================================
 *
 *       Filename:  test_Scheduler.cpp
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
#include "test_LoadBalancer.hpp"
#include <iostream>
#include <string>
#include <list>
#include <sdpa/memory.hpp>
#include <time.h>
#include <sdpa/util/util.hpp>
#include <fstream>
#include <sdpa/daemon/jobFSM/SMC/JobFSM.hpp>
#include <sdpa/daemon/WorkerManager.hpp>
#include <sstream>

using namespace std;
using namespace sdpa::tests;
using namespace sdpa::daemon;
using namespace sdpa::fsm::smc;

const int NWORKERS = 10;

CPPUNIT_TEST_SUITE_REGISTRATION( LoadBalancerTest );

LoadBalancerTest::LoadBalancerTest() : SDPA_INIT_LOGGER("sdpa.tests.LoadBalancerTest")
{

}

LoadBalancerTest::~LoadBalancerTest()
{
}

void LoadBalancerTest::setUp()
{
	SDPA_LOG_DEBUG("setUP");
	//initialize and start the finite state machine
}

void LoadBalancerTest::tearDown()
{
	SDPA_LOG_DEBUG("tearDown");
}

void LoadBalancerTest::testLoadBalancer()
{
	WorkerManager wm;
	ostringstream oss;
	size_t arrLoad[] = {100, 200, 10, 2, 30, 1000, 5, 300, 55, 701};

	// create a number of workers
	for(int k=0;k<NWORKERS;k++)
	{
		oss.str("");
		oss<<"Worker "<<k;
		wm.addWorker(oss.str(), k);
	}

	// submit jobs to the workers
	for(int k=0; k<NWORKERS; k++)
	{
		try {
			const Worker::ptr_t& ptrCurrWorker = wm.getNextWorker();

			for( size_t l=0; l<arrLoad[k]; l++)
			{
				sdpa::job_id_t jobId;
				ptrCurrWorker->dispatch(jobId);
			}


		}
		catch(const NoWorkerFoundException& ex) {
			SDPA_LOG_ERROR("No worker found!");
			CPPUNIT_ASSERT (false);
			break;
		}
	}

	// print loads before balancing
	for(int k=0; k<NWORKERS; k++)
	{
		try {
			const Worker::ptr_t& ptrCurrWorker = wm.getNextWorker();
			cout<<"load["<<ptrCurrWorker->name()<<"] = "<<ptrCurrWorker->pending().size()<<std::endl;
		}
		catch(const NoWorkerFoundException& ex) {
			SDPA_LOG_ERROR("No worker found!");
			CPPUNIT_ASSERT (false);
			break;
		}
	}

	cout<<"Balance the workers now ..."<<endl<<endl;
	wm.balanceWorkers();

	// print load after balancing
	for(int k=0; k<NWORKERS; k++)
	{
		try {
			const Worker::ptr_t& ptrCurrWorker = wm.getNextWorker();
			cout<<"load["<<ptrCurrWorker->name()<<"] = "<<ptrCurrWorker->pending().size()<<std::endl;
		}
		catch(const NoWorkerFoundException& ex) {
			SDPA_LOG_ERROR("No worker found!");
			CPPUNIT_ASSERT (false);
			break;
		}
	}
}

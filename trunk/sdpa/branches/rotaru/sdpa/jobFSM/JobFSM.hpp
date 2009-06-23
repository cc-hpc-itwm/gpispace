#ifndef JobFSM_H
#define JobFSM_H

#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/custom_reaction.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/exception_translator.hpp>

//Include the job related events here
#include <sdpa/events/RunJobEvent.hpp>
#include <sdpa/events/JobFailedEvent.hpp>
#include <sdpa/events/QueryJobStatusEvent.hpp>
#include <sdpa/events/JobStatusAnswerEvent.hpp>
#include <sdpa/events/CancelJobEvent.hpp>
#include <sdpa/events/CancelJobAckEvent.hpp>
#include <sdpa/events/JobFinishedEvent.hpp>
#include <sdpa/events/RetriveResultsEvent.hpp>

namespace mpl = boost::mpl;
namespace sc = boost::statechart;

using namespace std;
using namespace sdpa::events;

// FSM states (forward declarations)
struct Pending;
struct Running;
struct Terminating;
struct Terminated;
struct Cancelled;
struct Failed;
struct Finished;


// The FSM
struct JobFSM : sc::state_machine<JobFSM, Pending>
{
	void action_dispatch(const RunJobEvent& e);
	void action_cancel(const CancelJobEvent& e);
	void action_query_status(const QueryJobStatusEvent& e);
	void action_job_failed(const JobFailedEvent& e);
	void action_job_finished(const JobFinishedEvent& e );
	void action_retrieve_results(const RetriveResultsEvent& e );

	void WFE_NotifyNewJob();
	void WFE_GenListNextActiveSubJobs(); //assign unique global IDs!
	void WFE_NotifyJobFailed();

	void printStates();

private:

};

struct Pending : sc::simple_state<Pending, JobFSM>
{
	typedef mpl::list< sc::custom_reaction<RunJobEvent>,
					   sc::custom_reaction<CancelJobEvent>,
					   sc::custom_reaction<QueryJobStatusEvent>,
					   sc::custom_reaction<sc::exception_thrown> > reactions;

	Pending() { cout <<" enter state 'Pending'" << endl; }
	~Pending() { cout <<" leave state 'Pending'" << endl; }

	sc::result react( const RunJobEvent & e);
	sc::result react( const CancelJobEvent & e);
	sc::result react( const QueryJobStatusEvent & e);
	sc::result react( const sc::exception_thrown & e);
};

struct Running : sc::simple_state<Running, JobFSM>
{
typedef mpl::list< sc::custom_reaction<JobFinishedEvent>,
                   sc::custom_reaction<JobFailedEvent>,
                   sc::custom_reaction<CancelJobEvent>,
                   sc::custom_reaction<QueryJobStatusEvent>,
                   sc::custom_reaction<sc::exception_thrown> > reactions;

	Running() { cout << " enter state 'Running'" << endl; }
	~Running() { cout << " leave state 'Running'" << endl; }

	sc::result react( const JobFinishedEvent& );
  	sc::result react( const JobFailedEvent& );
    sc::result react( const CancelJobEvent& );
    sc::result react( const QueryJobStatusEvent& );
    sc::result react( const sc::exception_thrown & );
};

struct Cancelled : sc::simple_state<Cancelled, JobFSM, Terminating>
{
	typedef mpl::list< sc::custom_reaction< sc::exception_thrown > > reactions;
	Cancelled() { cout << " enter state 'Cancelled'" << endl; }
	~Cancelled() { cout << " leave state 'Cancelled'" << endl; }

	void action_query_status(const QueryJobStatusEvent& e);
	void action_cancel_ack(const CancelJobAckEvent& e);
	sc::result react( const sc::exception_thrown & e);
};

struct Terminating : sc::simple_state<Terminating, Cancelled>
{
typedef mpl::list< sc::custom_reaction<CancelJobAckEvent>,
                   sc::custom_reaction<QueryJobStatusEvent>,
                   sc::custom_reaction<sc::exception_thrown> > reactions;

	Terminating() { cout <<" enter state 'Terminating'" << endl; }
	~Terminating() { cout << " leave state 'Terminating'" << endl; }

    sc::result react( const CancelJobAckEvent& );
	sc::result react( const QueryJobStatusEvent& );
	sc::result react( const sc::exception_thrown & );
};

struct Terminated : sc::simple_state<Terminated, Cancelled>
{
typedef mpl::list< sc::custom_reaction<QueryJobStatusEvent>,
                   sc::custom_reaction<sc::exception_thrown> > reactions;

	Terminated() { cout << " enter state 'Terminated'" << endl; }
	~Terminated() { cout << " leave state 'Terminated'" << endl; }

    sc::result react( const QueryJobStatusEvent& );
    sc::result react( const sc::exception_thrown & );
};

struct Failed : sc::simple_state<Failed, JobFSM>
{
typedef mpl::list< sc::custom_reaction<QueryJobStatusEvent>,
                   sc::custom_reaction< sc::exception_thrown > > reactions;

	Failed() { cout << " enter state 'Failed'" << endl; }
	~Failed() { cout << " leave state 'Failed'" << endl; }

	sc::result react( const QueryJobStatusEvent& );
	sc::result react( const sc::exception_thrown & );
};

struct Finished : sc::simple_state<Finished, JobFSM>
{
typedef mpl::list< sc::custom_reaction<QueryJobStatusEvent>,
				   sc::custom_reaction<RetriveResultsEvent>,
                   sc::custom_reaction< sc::exception_thrown > > reactions;

	Finished() { cout << " enter state 'Finished'" << endl; }
	~Finished() { cout << " leave state 'Finished'" << endl; }

	sc::result react( const QueryJobStatusEvent& );
	sc::result react( const RetriveResultsEvent& );
	sc::result react( const sc::exception_thrown & );
};

#endif // JobFSM_H

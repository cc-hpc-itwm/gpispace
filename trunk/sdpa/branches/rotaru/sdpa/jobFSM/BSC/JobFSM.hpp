#ifndef JOBFSMBSC_H
#define JOBFSMBSC_H

#include <sdpa/jobFSM/JobFSMInterface.hpp>

#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/custom_reaction.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/exception_translator.hpp>

namespace mpl = boost::mpl;
namespace sc = boost::statechart;

using namespace std;

// FSM states (forward declarations)
struct Pending;
struct Running;
struct Terminating;
struct Terminated;
struct Cancelled;
struct Failed;
struct Finished;


// The FSM
struct JobFSM : public sdpa::fsm::JobFSMInterface, public sc::state_machine<JobFSM, Pending>
{
};

struct Pending : sc::simple_state<Pending, JobFSM>
{
	typedef mpl::list< sc::custom_reaction<sdpa::events::RunJobEvent>,
					   sc::custom_reaction<sdpa::events::CancelJobEvent>,
					   sc::custom_reaction<sdpa::events::QueryJobStatusEvent>,
					   sc::custom_reaction<sc::exception_thrown> > reactions;

	Pending() { cout <<" enter state 'Pending'" << endl; }
	~Pending() { cout <<" leave state 'Pending'" << endl; }

	sc::result react( const sdpa::events::RunJobEvent & e);
	sc::result react( const sdpa::events::CancelJobEvent & e);
	sc::result react( const sdpa::events::QueryJobStatusEvent & e);
	sc::result react( const sc::exception_thrown & e);
};

struct Running : sc::simple_state<Running, JobFSM>
{
typedef mpl::list< sc::custom_reaction<sdpa::events::JobFinishedEvent>,
                   sc::custom_reaction<sdpa::events::JobFailedEvent>,
                   sc::custom_reaction<sdpa::events::CancelJobEvent>,
                   sc::custom_reaction<sdpa::events::QueryJobStatusEvent>,
                   sc::custom_reaction<sc::exception_thrown> > reactions;

	Running() { cout << " enter state 'Running'" << endl; }
	~Running() { cout << " leave state 'Running'" << endl; }

	sc::result react( const sdpa::events::JobFinishedEvent& );
  	sc::result react( const sdpa::events::JobFailedEvent& );
    sc::result react( const sdpa::events::CancelJobEvent& );
    sc::result react( const sdpa::events::QueryJobStatusEvent& );
    sc::result react( const sc::exception_thrown & );
};

struct Cancelled : sc::simple_state<Cancelled, JobFSM, Terminating>
{
	typedef mpl::list< sc::custom_reaction< sc::exception_thrown > > reactions;
	Cancelled() { cout << " enter state 'Cancelled'" << endl; }
	~Cancelled() { cout << " leave state 'Cancelled'" << endl; }

	void action_query_status(const sdpa::events::QueryJobStatusEvent& e);
	void action_cancel_ack(const sdpa::events::CancelJobAckEvent& e);
	sc::result react( const sc::exception_thrown & e);
};

struct Terminating : sc::simple_state<Terminating, Cancelled>
{
typedef mpl::list< sc::custom_reaction<sdpa::events::CancelJobAckEvent>,
                   sc::custom_reaction<sdpa::events::QueryJobStatusEvent>,
                   sc::custom_reaction<sc::exception_thrown> > reactions;

	Terminating() { cout <<" enter state 'Terminating'" << endl; }
	~Terminating() { cout << " leave state 'Terminating'" << endl; }

    sc::result react( const sdpa::events::CancelJobAckEvent& );
	sc::result react( const sdpa::events::QueryJobStatusEvent& );
	sc::result react( const sc::exception_thrown & );
};

struct Terminated : sc::simple_state<Terminated, Cancelled>
{
typedef mpl::list< sc::custom_reaction<sdpa::events::QueryJobStatusEvent>,
                   sc::custom_reaction<sc::exception_thrown> > reactions;

	Terminated() { cout << " enter state 'Terminated'" << endl; }
	~Terminated() { cout << " leave state 'Terminated'" << endl; }

    sc::result react( const sdpa::events::QueryJobStatusEvent& );
    sc::result react( const sc::exception_thrown & );
};

struct Failed : sc::simple_state<Failed, JobFSM>
{
typedef mpl::list< sc::custom_reaction<sdpa::events::QueryJobStatusEvent>,
                   sc::custom_reaction< sc::exception_thrown > > reactions;

	Failed() { cout << " enter state 'Failed'" << endl; }
	~Failed() { cout << " leave state 'Failed'" << endl; }

	sc::result react( const sdpa::events::QueryJobStatusEvent& );
	sc::result react( const sc::exception_thrown & );
};

struct Finished : sc::simple_state<Finished, JobFSM>
{
typedef mpl::list< sc::custom_reaction<sdpa::events::QueryJobStatusEvent>,
				   sc::custom_reaction<sdpa::events::RetriveResultsEvent>,
                   sc::custom_reaction< sc::exception_thrown > > reactions;

	Finished() { cout << " enter state 'Finished'" << endl; }
	~Finished() { cout << " leave state 'Finished'" << endl; }

	sc::result react( const sdpa::events::QueryJobStatusEvent& );
	sc::result react( const sdpa::events::RetriveResultsEvent& );
	sc::result react( const sc::exception_thrown & );
};

#endif // JobFSM_H

#ifndef JOBFSMBSC_H
#define JOBFSMBSC_H

#include <sdpa/jobFSM/JobFSMActions.hpp>

#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/custom_reaction.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/exception_translator.hpp>

namespace mpl = boost::mpl;
namespace sc = boost::statechart;

//using namespace std;

namespace sdpa { namespace fsm { namespace bsc {


// FSM states (forward declarations)
struct Pending;
struct Running;
struct Terminating;
struct Terminated;
struct Cancelled;
struct Failed;
struct Finished;


// The FSM
struct JobFSM : public sdpa::fsm::JobFSMActions, public sc::state_machine<JobFSM, Pending>
{
};

struct Pending : sc::simple_state<Pending, JobFSM>
{
	typedef mpl::list< sc::custom_reaction<sdpa::events::RunJobEvent>,
					   sc::custom_reaction<sdpa::events::CancelJobEvent>,
					   sc::custom_reaction<sdpa::events::QueryJobStatusEvent>,
					   sc::custom_reaction<sc::exception_thrown> > reactions;

	Pending() { std::cout<<" enter state 'Pending'" << std::endl; }
	~Pending() { std::cout<<" leave state 'Pending'" << std::endl; }

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

	Running() { std::cout<< " enter state 'Running'" << std::endl; }
	~Running() { std::cout<< " leave state 'Running'" << std::endl; }

	sc::result react( const sdpa::events::JobFinishedEvent& );
  	sc::result react( const sdpa::events::JobFailedEvent& );
    sc::result react( const sdpa::events::CancelJobEvent& );
    sc::result react( const sdpa::events::QueryJobStatusEvent& );
    sc::result react( const sc::exception_thrown & );
};

struct Cancelled : sc::simple_state<Cancelled, JobFSM, Terminating>
{
	typedef mpl::list< sc::custom_reaction< sc::exception_thrown > > reactions;
	Cancelled() { std::cout<< " enter state 'Cancelled'" << std::endl; }
	~Cancelled() { std::cout<< " leave state 'Cancelled'" << std::endl; }

	void action_query_status(const sdpa::events::QueryJobStatusEvent& e);
	void action_cancel_ack(const sdpa::events::CancelJobAckEvent& e);
	sc::result react( const sc::exception_thrown & e);
};

struct Terminating : sc::simple_state<Terminating, Cancelled>
{
typedef mpl::list< sc::custom_reaction<sdpa::events::CancelJobAckEvent>,
                   sc::custom_reaction<sdpa::events::QueryJobStatusEvent>,
                   sc::custom_reaction<sc::exception_thrown> > reactions;

	Terminating() { std::cout<<" enter state 'Terminating'" << std::endl; }
	~Terminating() { std::cout<< " leave state 'Terminating'" << std::endl; }

    sc::result react( const sdpa::events::CancelJobAckEvent& );
	sc::result react( const sdpa::events::QueryJobStatusEvent& );
	sc::result react( const sc::exception_thrown & );
};

struct Terminated : sc::simple_state<Terminated, Cancelled>
{
typedef mpl::list< sc::custom_reaction<sdpa::events::QueryJobStatusEvent>,
                   sc::custom_reaction<sc::exception_thrown> > reactions;

	Terminated() { std::cout<< " enter state 'Terminated'" << std::endl; }
	~Terminated() { std::cout<< " leave state 'Terminated'" << std::endl; }

    sc::result react( const sdpa::events::QueryJobStatusEvent& );
    sc::result react( const sc::exception_thrown & );
};

struct Failed : sc::simple_state<Failed, JobFSM>
{
typedef mpl::list< sc::custom_reaction<sdpa::events::QueryJobStatusEvent>,
                   sc::custom_reaction< sc::exception_thrown > > reactions;

	Failed() { std::cout<< " enter state 'Failed'" << std::endl; }
	~Failed() { std::cout<< " leave state 'Failed'" << std::endl; }

	sc::result react( const sdpa::events::QueryJobStatusEvent& );
	sc::result react( const sc::exception_thrown & );
};

struct Finished : sc::simple_state<Finished, JobFSM>
{
typedef mpl::list< sc::custom_reaction<sdpa::events::QueryJobStatusEvent>,
				   sc::custom_reaction<sdpa::events::RetriveResultsEvent>,
                   sc::custom_reaction< sc::exception_thrown > > reactions;

	Finished() { std::cout<< " enter state 'Finished'" << std::endl; }
	~Finished() { std::cout<< " leave state 'Finished'" << std::endl; }

	sc::result react( const sdpa::events::QueryJobStatusEvent& );
	sc::result react( const sdpa::events::RetriveResultsEvent& );
	sc::result react( const sc::exception_thrown & );
};

}}}

#endif // JobFSM_H

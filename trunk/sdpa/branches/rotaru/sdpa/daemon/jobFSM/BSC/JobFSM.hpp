#ifndef JOB_FSM_BSC_HPP
#define JOB_FSM_BSC_HPP 1

#include <sdpa/daemon/JobImpl.hpp>

#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/simple_state.hpp>
#include <boost/statechart/custom_reaction.hpp>
#include <boost/statechart/transition.hpp>
#include <boost/statechart/exception_translator.hpp>

#include <sdpa/logging.hpp>

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
struct JobFSM : public sdpa::daemon::JobImpl, public sc::state_machine<JobFSM, Pending>
{
	JobFSM( const sdpa::job_id_t &id,
			const sdpa::job_desc_t &desc,
			const sdpa::daemon::ISendEvent* pHandler = NULL,
			const sdpa::job_id_t &parent = Job::invalid_job_id());

	virtual ~JobFSM()  throw ();
	virtual void process_event( const boost::statechart::event_base & e) {
		 sc::state_machine<JobFSM, Pending>::process_event(e);
    }

	void CancelJob(const sdpa::events::CancelJobEvent& event){ process_event(event); }
	void CancelJobAck(const sdpa::events::CancelJobAckEvent& event){ process_event(event); }
	void DeleteJob(const sdpa::events::DeleteJobEvent& event){ process_event(event); }
	void JobFailed(const sdpa::events::JobFailedEvent& event){ process_event(event); }
	void JobFinished(const sdpa::events::JobFinishedEvent& event){ process_event(event); }
	void QueryJobStatus(const sdpa::events::QueryJobStatusEvent& event){ process_event(event); }
	void RetrieveJobResults(const sdpa::events::RetrieveJobResultsEvent& event){ process_event(event); }
	void RunJob(const sdpa::events::RunJobEvent& event){ process_event(event); }

	void print_states();
private:
	SDPA_DECLARE_LOGGER();
};

struct Pending : sc::simple_state<Pending, JobFSM>
{
	typedef mpl::list< sc::custom_reaction<sdpa::events::RunJobEvent>,
					   sc::custom_reaction<sdpa::events::CancelJobEvent>,
					   sc::custom_reaction<sdpa::events::QueryJobStatusEvent>,
					   sc::custom_reaction<sc::exception_thrown> > reactions;

	Pending() { } //std::cout<<" enter state 'Pending'" << std::endl; }
	~Pending() { } //std::cout<<" leave state 'Pending'" << std::endl; }

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

	Running() { } //std::cout<< " enter state 'Running'" << std::endl; }
	~Running() { } //std::cout<< " leave state 'Running'" << std::endl; }

	sc::result react( const sdpa::events::JobFinishedEvent& );
  	sc::result react( const sdpa::events::JobFailedEvent& );
    sc::result react( const sdpa::events::CancelJobEvent& );
    sc::result react( const sdpa::events::QueryJobStatusEvent& );
    sc::result react( const sc::exception_thrown & );
};

struct Cancelled : sc::simple_state<Cancelled, JobFSM, Terminating>
{
	typedef mpl::list< sc::custom_reaction< sc::exception_thrown > > reactions;
	Cancelled() { } //std::cout<< " enter state 'Cancelled'" << std::endl; }
	~Cancelled() { } //std::cout<< " leave state 'Cancelled'" << std::endl; }

	void action_query_status(const sdpa::events::QueryJobStatusEvent& e);
	void action_cancel_ack(const sdpa::events::CancelJobAckEvent& e);
	sc::result react( const sc::exception_thrown & e);
};

struct Terminating : sc::simple_state<Terminating, Cancelled>
{
typedef mpl::list< sc::custom_reaction<sdpa::events::CancelJobAckEvent>,
                   sc::custom_reaction<sdpa::events::QueryJobStatusEvent>,
                   sc::custom_reaction<sc::exception_thrown> > reactions;

	Terminating() { } //std::cout<<" enter state 'Terminating'" << std::endl; }
	~Terminating() { } //std::cout<< " leave state 'Terminating'" << std::endl; }

    sc::result react( const sdpa::events::CancelJobAckEvent& );
	sc::result react( const sdpa::events::QueryJobStatusEvent& );
	sc::result react( const sc::exception_thrown & );
};

struct Terminated : sc::simple_state<Terminated, Cancelled>
{
typedef mpl::list< 	sc::custom_reaction<sdpa::events::QueryJobStatusEvent>,
					sc::custom_reaction<sdpa::events::DeleteJobEvent>,
                    sc::custom_reaction<sc::exception_thrown> > reactions;

	Terminated() { } //std::cout<< " enter state 'Terminated'" << std::endl; }
	~Terminated() { } //std::cout<< " leave state 'Terminated'" << std::endl; }

    sc::result react( const sdpa::events::QueryJobStatusEvent& );
    sc::result react( const sdpa::events::DeleteJobEvent& );
    sc::result react( const sc::exception_thrown & );
};

struct Failed : sc::simple_state<Failed, JobFSM>
{
typedef mpl::list< 	sc::custom_reaction<sdpa::events::QueryJobStatusEvent>,
					sc::custom_reaction<sdpa::events::DeleteJobEvent>,
					sc::custom_reaction< sc::exception_thrown > > reactions;

	Failed() { } //std::cout<< " enter state 'Failed'" << std::endl; }
	~Failed() { } //std::cout<< " leave state 'Failed'" << std::endl; }

	sc::result react( const sdpa::events::QueryJobStatusEvent& );
	sc::result react( const sdpa::events::DeleteJobEvent& );
	sc::result react( const sc::exception_thrown & );
};

struct Finished : sc::simple_state<Finished, JobFSM>
{
typedef mpl::list<  sc::custom_reaction<sdpa::events::QueryJobStatusEvent>,
					sc::custom_reaction<sdpa::events::DeleteJobEvent>,
					sc::custom_reaction<sdpa::events::RetrieveJobResultsEvent>,
					sc::custom_reaction< sc::exception_thrown > > reactions;

	Finished() { } //std::cout<< " enter state 'Finished'" << std::endl; }
	~Finished() { } //std::cout<< " leave state 'Finished'" << std::endl; }

	sc::result react( const sdpa::events::QueryJobStatusEvent& );
	sc::result react( const sdpa::events::DeleteJobEvent& );
	sc::result react( const sdpa::events::RetrieveJobResultsEvent& );
	sc::result react( const sc::exception_thrown & );
};

}}}

#endif // JobFSM_H

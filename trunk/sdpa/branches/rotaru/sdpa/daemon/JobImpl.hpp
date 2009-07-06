#ifndef SDPA_JOB_IMPL_HPP
#define SDPA_JOB_IMPL_HPP 1

#include <sdpa/daemon/Job.hpp>
#include <sdpa/daemon/JobFSMActions.hpp>
#include <sdpa/daemon/ISendEvent.hpp>
#include <sdpa/common.hpp>
#include <map>

namespace sdpa { namespace daemon {
    class JobImpl : public Job, public sdpa::fsm::JobFSMActions  {
    public:
        typedef std::map<sdpa::job_id_t, Job::ptr_t> job_list_t;
        typedef sdpa::shared_ptr<JobImpl> ptr_t;

        JobImpl(const sdpa::job_id_t &id,
                const sdpa::job_desc_t &desc,
                const sdpa::daemon::ISendEvent* pHandler = NULL,
                const sdpa::job_id_t &parent = Job::invalid_job_id());

        virtual ~JobImpl() throw();

        virtual const sdpa::job_id_t & id() const;
        virtual const sdpa::job_id_t & parent() const;

        virtual const sdpa::job_desc_t & description() const;

        virtual const Job::data_t & input() const;
        virtual const Job::data_t & output() const;

        virtual void add_input(const Job::value_t & value);
        virtual void add_output(const Job::value_t & value);

        virtual void add_subjob(const Job::ptr_t & job);
        virtual Job::ptr_t get_subjob(const job_id_t & id);

        virtual bool is_marked_for_deletion();
    	virtual void process_event( const boost::statechart::event_base & e);

        //job FSM actions
		virtual void action_run_job(const sdpa::events::RunJobEvent& e);
		virtual void action_cancel_job(const sdpa::events::CancelJobEvent& e);
		virtual void action_cancel_job_ack(const sdpa::events::CancelJobAckEvent& e);
		virtual void action_delete_job(const sdpa::events::DeleteJobEvent& e);
		virtual void action_query_job_status(const sdpa::events::QueryJobStatusEvent& e);
		virtual void action_job_failed(const sdpa::events::JobFailedEvent& e);
		virtual void action_job_finished(const sdpa::events::JobFinishedEvent& e );
		virtual void action_retrieve_job_results(const sdpa::events::RetrieveResultsEvent& e );

		virtual void CancelJob(const sdpa::events::CancelJobEvent& event);
		virtual void CancelJobAck(const sdpa::events::CancelJobAckEvent& event);
		virtual void DeleteJob(const sdpa::events::DeleteJobEvent& event);
		virtual void JobFailed(const sdpa::events::JobFailedEvent& event);
		virtual void JobFinished(const sdpa::events::JobFinishedEvent& event);
		virtual void QueryJobStatus(const sdpa::events::QueryJobStatusEvent& event);
		virtual void RetrieveResults(const sdpa::events::RetrieveResultsEvent& event);
		virtual void RunJob(const sdpa::events::RunJobEvent& event);

    private:
        sdpa::job_id_t id_;
        sdpa::job_desc_t desc_;
        sdpa::job_id_t parent_;

        Job::data_t input_;
        Job::data_t output_;
        job_list_t subjobs_;

        bool b_marked_for_del_;
        SDPA_DECLARE_LOGGER();
    protected:
       	const ISendEvent* pSendEvent;
    };
}}

#endif

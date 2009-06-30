#ifndef SDPA_JOB_IMPL_HPP
#define SDPA_JOB_IMPL_HPP 1

#include <sdpa/daemon/Job.hpp>
#include <sdpa/daemon/JobFSMActions.hpp>

#include <map>

namespace sdpa { namespace daemon {
    class JobImpl : public Job, public sdpa::fsm::JobFSMActions  {
    public:
        typedef std::map<Job::job_id_t, Job::ptr_t> job_list_t;

        JobImpl(const Job::job_id_t &id,
                const Job::job_desc_t &desc,
                const Job::job_id_t &parent = Job::invalid_job_id());

        virtual ~JobImpl() throw();

        virtual const Job::job_id_t & id() const;
        virtual const Job::job_id_t & parent() const;

        virtual const Job::job_desc_t & description() const;

        virtual const Job::data_t & input() const;
        virtual const Job::data_t & output() const;

        virtual void add_input(const Job::value_t & value);
        virtual void add_output(const Job::value_t & value);

        virtual void add_subjob(const Job::ptr_t & job);
        virtual Job::ptr_t get_subjob(const job_id_t & id);

        //job FSM actions
		virtual void action_run_job(const sdpa::events::RunJobEvent& e);
		virtual void action_cancel_job(const sdpa::events::CancelJobEvent& e);
		virtual void action_cancel_job_ack(const sdpa::events::CancelJobAckEvent& e);
		virtual void action_query_job_status(const sdpa::events::QueryJobStatusEvent& e);
		virtual void action_job_failed(const sdpa::events::JobFailedEvent& e);
		virtual void action_job_finished(const sdpa::events::JobFinishedEvent& e );
		virtual void action_retrieve_job_results(const sdpa::events::RetriveResultsEvent& e );
		virtual void WFE_NotifyNewJob();
		virtual void WFE_GenListNextActiveSubJobs(); //assign unique global IDs!
		virtual void WFE_NotifyJobFailed();

    private:
        Job::job_id_t id_;
        Job::job_desc_t desc_;
        Job::job_id_t parent_;

        Job::data_t input_;
        Job::data_t output_;
        job_list_t subjobs_;
    };
}}

#endif

#ifndef SDPA_JOB_IMPL_HPP
#define SDPA_JOB_IMPL_HPP 1

#include <sdpa/daemon/Job.hpp>
#include <sdpa/daemon/JobFSMActions.hpp>
#include <sdpa/daemon/ISendEvent.hpp>
#include <sdpa/common.hpp>
#include <map>
#include <boost/thread.hpp>


namespace sdpa { namespace daemon {
    class JobImpl : public Job, public sdpa::fsm::JobFSMActions  {
    public:
        typedef std::map<sdpa::job_id_t, Job::ptr_t> job_list_t;
        typedef sdpa::shared_ptr<JobImpl> ptr_t;

        typedef boost::recursive_mutex mutex_type;
      	typedef boost::unique_lock<mutex_type> lock_type;

        JobImpl(const sdpa::job_id_t &id,
                const sdpa::job_desc_t &desc,
                const sdpa::daemon::IComm* pHandler = NULL,
                const sdpa::job_id_t &parent = Job::invalid_job_id());

        virtual ~JobImpl() throw();

        virtual const sdpa::job_id_t& id() const;
        virtual const sdpa::job_id_t& parent() const;

        virtual const sdpa::job_desc_t& description() const;

        virtual const Job::data_t& input() const;
        virtual const Job::data_t& output() const;

        virtual void add_input(const Job::value_t & value);
        virtual void add_output(const Job::value_t & value);

        virtual void add_subjob(const Job::ptr_t & job);
        virtual Job::ptr_t get_subjob(const job_id_t & id);

        virtual bool is_marked_for_deletion();
        virtual bool mark_for_deletion();

        virtual bool is_local();
        virtual void set_local(bool);

        // job FSM actions
		virtual void action_run_job();
		virtual void action_cancel_job(const sdpa::events::CancelJobEvent&);
		virtual void action_cancel_job_from_pending(const sdpa::events::CancelJobEvent&);
		virtual void action_cancel_job_ack(const sdpa::events::CancelJobAckEvent&);
		virtual void action_delete_job(const sdpa::events::DeleteJobEvent&);
		virtual void action_query_job_status(const sdpa::events::QueryJobStatusEvent&);
		virtual void action_job_failed(const sdpa::events::JobFailedEvent&);
		virtual void action_job_finished(const sdpa::events::JobFinishedEvent&);
		virtual void action_retrieve_job_results(const sdpa::events::RetrieveJobResultsEvent&);

    private:
        sdpa::job_id_t id_;
        sdpa::job_desc_t desc_;
        sdpa::job_id_t parent_;

        Job::data_t input_;
        Job::data_t output_;
        job_list_t  subjobs_;

        bool b_marked_for_del_;
        bool b_local_;
        SDPA_DECLARE_LOGGER();
    protected:
       	mutable IComm* pComm;
        mutex_type mtx_;
    };
}}

#endif

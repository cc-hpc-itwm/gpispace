/*
 * =====================================================================================
 *
 *       Filename:  JobImpl.hpp
 *
 *    Description:  Job implementation header
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
#ifndef SDPA_JOB_IMPL_HPP
#define SDPA_JOB_IMPL_HPP 1
#include <sdpa/daemon/Job.hpp>
#include <sdpa/daemon/JobFSMActions.hpp>
#include <sdpa/daemon/IComm.hpp>
#include <sdpa/common.hpp>
#include <boost/thread.hpp>

#include <boost/unordered_map.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/access.hpp>

namespace sdpa { namespace daemon {
    class JobImpl : public Job /*, public sdpa::fsm::JobFSMActions*/  {
    public:
        typedef boost::unordered_map<sdpa::job_id_t, Job::ptr_t> job_list_t;
        typedef sdpa::shared_ptr<JobImpl> ptr_t;

        typedef boost::recursive_mutex mutex_type;
      	typedef boost::unique_lock<mutex_type> lock_type;

      	JobImpl(const sdpa::job_id_t id = JobId(""),
      	                const sdpa::job_desc_t desc = "",
      	                const sdpa::daemon::IComm* pHandler = NULL,
      	                const sdpa::job_id_t &parent = sdpa::job_id_t::invalid_job_id());

       virtual ~JobImpl();

        virtual const sdpa::job_id_t& id() const;
        virtual const sdpa::job_id_t& parent() const;
        virtual const sdpa::job_desc_t& description() const;
        virtual void set_icomm(IComm* pArgComm) { pComm = pArgComm; }

        //virtual sdpa::worker_id_t& worker() { return worker_id_;}

        virtual bool is_marked_for_deletion();
        virtual bool mark_for_deletion();

        virtual bool is_local();
        virtual void set_local(bool);

        virtual unsigned long &walltime() { return walltime_;}

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

		virtual void setResult(const sdpa::job_result_t& arg_results) { result = arg_results; }


		virtual std::string print_info()
		{
			std::ostringstream os;
			os<<id_<<std::endl;
			os<<desc_<<std::endl;
			os<<parent_<<std::endl;
			//os<<worker_id_<<std::endl;
			return os.str();
		}

		template <class Archive> void serialize(Archive& ar, const unsigned int)
		{
			ar & boost::serialization::base_object<Job>(*this);
			ar & id_;
			ar & desc_;
			ar & parent_;
			//ar & worker_id_;
			ar & result;
			ar & walltime_;
		}

    protected:
        SDPA_DECLARE_LOGGER();

    private:
        sdpa::job_id_t id_;
        sdpa::job_desc_t desc_;
        sdpa::job_id_t parent_;

        bool b_marked_for_del_;
        bool b_local_;
        sdpa::job_result_t result;

        friend class boost::serialization::access;
        //sdpa::worker_id_t worker_id_;
        unsigned long walltime_;
    protected:
       	/*mutable*/ IComm* pComm;
        mutex_type mtx_;

    };
}}

#endif

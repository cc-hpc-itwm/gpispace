#ifndef SDPA_JOB_HPP
#define SDPA_JOB_HPP 1

#include <sdpa/daemon/mpl.hpp>
#include <string>
#include <vector>
#include <utility>

#include <sdpa/memory.hpp>
#include <sdpa/types.hpp>
#include <sdpa/util/Properties.hpp>

#include <sdpa/events/SubmitJobAckEvent.hpp>
#include <sdpa/events/JobFailedEvent.hpp>
#include <sdpa/events/QueryJobStatusEvent.hpp>
#include <sdpa/events/JobStatusReplyEvent.hpp>
#include <sdpa/events/CancelJobEvent.hpp>
#include <sdpa/events/CancelJobAckEvent.hpp>
#include <sdpa/events/DeleteJobEvent.hpp>
#include <sdpa/events/JobFinishedEvent.hpp>
#include <sdpa/events/ErrorEvent.hpp>
#include <sdpa/events/RetrieveJobResultsEvent.hpp>
#include <sdpa/types.hpp>

#include <sdpa/common.hpp>
#include <boost/thread.hpp>

#include <boost/unordered_map.hpp>

namespace sdpa {
  namespace daemon {
    class IAgent;
    class Job
    {
    public:
      typedef Job* ptr_t;

      enum job_type {MASTER, LOCAL, WORKER, TMP};

      typedef boost::unordered_map<sdpa::job_id_t, Job::ptr_t> job_list_t;
      typedef boost::recursive_mutex mutex_type;
      typedef boost::unique_lock<mutex_type> lock_type;

      Job(const sdpa::job_id_t id = JobId(""),
              const sdpa::job_desc_t desc = "",
              const sdpa::daemon::IAgent* pHandler = NULL,
              const sdpa::job_id_t &parent = sdpa::job_id_t::invalid_job_id());

      virtual ~Job() {}

      virtual const sdpa::job_id_t& id() const;
      virtual const sdpa::job_id_t& parent() const;
      virtual const sdpa::job_desc_t& description() const;
      virtual const sdpa::job_result_t& result() const { return result_; }

      int error_code() const {return m_error_code;}
      std::string const & error_message () const { return m_error_message;}

      Job& error_code(int ec)
      {
        m_error_code = ec;
        return *this;
      }

      Job& error_message(std::string const &msg)
      {
        m_error_message = msg;
        return *this;
      }

      virtual bool is_marked_for_deletion();
      virtual bool mark_for_deletion();

      bool isMasterJob();
      void setType(const job_type& );
      virtual job_type type() { return type_;}

      virtual void set_owner(const sdpa::worker_id_t& owner) { m_owner = owner; }
      virtual sdpa::worker_id_t owner() { return m_owner; }

      virtual bool completed() { return false; }
      virtual bool is_running() { return false;};

      virtual unsigned long &walltime() { return walltime_;}

      // job FSM actions
      virtual void action_run_job() {}
      virtual void action_cancel_job(const sdpa::events::CancelJobEvent&) {}
      virtual void action_cancel_job_from_pending(const sdpa::events::CancelJobEvent&) {}
      virtual void action_cancel_job_ack(const sdpa::events::CancelJobAckEvent&) {}
      virtual void action_delete_job(const sdpa::events::DeleteJobEvent&);
      virtual void action_job_failed(const sdpa::events::JobFailedEvent&);
      virtual void action_job_finished(const sdpa::events::JobFinishedEvent&);
      virtual void action_retrieve_job_results(const sdpa::events::RetrieveJobResultsEvent&) {}

      virtual void setResult(const sdpa::job_result_t& arg_results) { result_ = arg_results; }

      virtual std::string print_info()
      {
        std::ostringstream os;
        os<<std::endl;
        os<<"id: "<<id_<<std::endl;
        os<<"type: "<<type_<<std::endl;
        os<<"status: "<<getStatus()<<std::endl;
        os<<"parent: "<<parent_<<std::endl;
        os<<"error-code: " << m_error_code << std::endl;
        os<<"error-message: \"" << m_error_message << "\"" << std::endl;
        //os<<"description: "<<desc_<<std::endl;

        return os.str();
      }

      //transitions (implemented in JobFSM)
      virtual void CancelJob(const sdpa::events::CancelJobEvent*) = 0;
      virtual void CancelJobAck(const sdpa::events::CancelJobAckEvent*) = 0;
      virtual void DeleteJob(const sdpa::events::DeleteJobEvent*, sdpa::daemon::IAgent*) = 0;
      virtual void JobFailed(const sdpa::events::JobFailedEvent*) = 0;
      virtual void JobFinished(const sdpa::events::JobFinishedEvent*) = 0;
      virtual void QueryJobStatus(const sdpa::events::QueryJobStatusEvent*, sdpa::daemon::IAgent* ) = 0;
      virtual void RetrieveJobResults(const sdpa::events::RetrieveJobResultsEvent*, sdpa::daemon::IAgent*) = 0;
      virtual void Dispatch() = 0;
      virtual void Reschedule(sdpa::daemon::IAgent*) = 0;
      virtual void Pause() = 0;

      virtual sdpa::status_t getStatus() = 0;

    protected:
      SDPA_DECLARE_LOGGER();

    private:
      sdpa::job_id_t id_;
      sdpa::job_desc_t desc_;
      sdpa::job_id_t parent_;

      bool b_marked_for_del_;
      job_type type_;
      sdpa::job_result_t result_;
      int m_error_code;
      std::string m_error_message;
      unsigned long walltime_;

      sdpa::worker_id_t m_owner;
    };
}}

#endif

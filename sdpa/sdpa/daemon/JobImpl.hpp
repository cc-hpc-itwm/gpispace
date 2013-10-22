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
#include <sdpa/daemon/IAgent.hpp>
#include <sdpa/common.hpp>
#include <boost/thread.hpp>

#include <boost/unordered_map.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/version.hpp>

namespace sdpa {
  namespace daemon {
    class JobImpl : public Job
    {
    public:
      typedef boost::unordered_map<sdpa::job_id_t, Job::ptr_t> job_list_t;
      typedef boost::recursive_mutex mutex_type;
      typedef boost::unique_lock<mutex_type> lock_type;

      JobImpl(const sdpa::job_id_t id = JobId(""),
              const sdpa::job_desc_t desc = "",
              const sdpa::daemon::IAgent* pHandler = NULL,
              const sdpa::job_id_t &parent = sdpa::job_id_t::invalid_job_id());

      virtual ~JobImpl();

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

      virtual void set_icomm(IAgent* pArgComm) { pComm = pArgComm; }
      virtual IAgent* icomm() { return pComm; }

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
      virtual void action_run_job();
      virtual void action_cancel_job(const sdpa::events::CancelJobEvent&);
      virtual void action_cancel_job_from_pending(const sdpa::events::CancelJobEvent&);
      virtual void action_cancel_job_ack(const sdpa::events::CancelJobAckEvent&);
      virtual void action_delete_job(const sdpa::events::DeleteJobEvent&);
      virtual void action_job_failed(const sdpa::events::JobFailedEvent&);
      virtual void action_job_finished(const sdpa::events::JobFinishedEvent&);
      virtual void action_retrieve_job_results(const sdpa::events::RetrieveJobResultsEvent&);

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

      template <class Archive> void serialize( Archive& ar, const unsigned int version)
      {
        ar & boost::serialization::base_object<Job>(*this);
        ar & id_;
        ar & desc_;
        ar & parent_;
        ar & result_;
        ar & walltime_;
        ar & type_;
        ar & m_owner;
        if (version > 0)
        {
          ar & m_error_code;
          ar & m_error_message;
        }
      }

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
      friend class boost::serialization::access;
      unsigned long walltime_;

      sdpa::worker_id_t m_owner;
    protected:
      IAgent* pComm;
    };
}}

BOOST_CLASS_VERSION(sdpa::daemon::JobImpl, 1);

#endif

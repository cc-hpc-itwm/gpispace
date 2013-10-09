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

#include <boost/serialization/access.hpp>

namespace sdpa {
  namespace daemon {

    class IAgent;
    class GenericDaemon;
    /**
     * The interface to the generic job description we keep around in all
     * components.
     */
    class Job /*: public sdpa::util::Properties */
    {
    public:
    //typedef sdpa::shared_ptr<Job> ptr_t;
    typedef Job* ptr_t;

    enum job_type {MASTER, LOCAL, WORKER, TMP};

    virtual ~Job() {}

    virtual const job_id_t & id() const = 0;
    virtual const job_id_t & parent() const = 0;
    virtual const job_desc_t & description() const = 0;
    virtual const sdpa::job_result_t& result() const = 0;

    virtual int error_code() const = 0;
    virtual std::string const & error_message() const = 0;

    virtual Job & error_code(int) = 0;
    virtual Job & error_message(std::string const &) = 0;

    virtual void set_icomm(IAgent* pArgComm) = 0;
    virtual IAgent* icomm() = 0;

    //virtual sdpa::worker_id_t& worker() = 0;

    virtual bool is_marked_for_deletion() = 0;
    virtual bool mark_for_deletion() = 0;

    virtual bool isMasterJob()=0;
    virtual void setType(const job_type& )=0;
    virtual job_type type()=0;

    virtual std::string print_info() = 0;

    virtual unsigned long& walltime() = 0;

    virtual void set_owner(const sdpa::worker_id_t& owner) = 0;
    virtual sdpa::worker_id_t owner() = 0;

    //transitions
    virtual void CancelJob(const sdpa::events::CancelJobEvent*);
    virtual void CancelJobAck(const sdpa::events::CancelJobAckEvent*);
    virtual void DeleteJob(const sdpa::events::DeleteJobEvent*, sdpa::daemon::IAgent*);
    virtual void JobFailed(const sdpa::events::JobFailedEvent*);
    virtual void JobFinished(const sdpa::events::JobFinishedEvent*);
    virtual void QueryJobStatus(const sdpa::events::QueryJobStatusEvent*, sdpa::daemon::IAgent* );
    virtual void RetrieveJobResults(const sdpa::events::RetrieveJobResultsEvent*, sdpa::daemon::IAgent*);
    virtual void Dispatch();
    virtual void Reschedule();

    virtual void setResult(const sdpa::job_result_t& ) =0;
    virtual sdpa::status_t getStatus() { return "Undefined"; }
    virtual bool completed() = 0;

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive&, const unsigned int /* file version */){}
  };
}}

BOOST_SERIALIZATION_ASSUME_ABSTRACT( sdpa::daemon::Job )

#endif

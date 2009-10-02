#ifndef SDPA_JOB_HPP
#define SDPA_JOB_HPP 1

#include <string>
#include <vector>
#include <utility>
#include <sdpa/memory.hpp>
#include <sdpa/types.hpp>
#include <sdpa/wf/types.hpp>
#include <sdpa/Properties.hpp>

#include <boost/statechart/event_base.hpp>
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

namespace sdpa { namespace daemon {

	class GenericDaemon;
    /**
     * The interface to the generic job description we keep around in all
     * components.
     */
    class Job : public Properties {
    public:
        typedef sdpa::shared_ptr<Job> ptr_t;

        typedef std::string token_t;
        typedef std::string place_t;
        typedef std::pair<place_t, token_t> value_t;
        typedef std::vector<value_t> data_t;

        static const job_id_t &invalid_job_id();

        virtual const job_id_t & id() const = 0;
        virtual const job_id_t & parent() const = 0;

        virtual const sdpa::wf::workflow_id_t& workflow_id() const = 0;
        virtual const job_desc_t & description() const = 0;

        virtual const data_t & input() const = 0;
        virtual const data_t & output() const = 0;

        virtual void add_input(const value_t & value) = 0;
        virtual void add_output(const value_t & value) = 0;

        virtual void add_subjob(const ptr_t & job) = 0;
        virtual ptr_t get_subjob(const job_id_t & id) = 0;

        virtual bool is_marked_for_deletion() = 0;
        virtual bool mark_for_deletion() = 0;

        virtual bool is_local()=0;
        virtual void set_local(bool)=0;
        virtual void process_event( const boost::statechart::event_base & e) {}

        //transitions
		virtual void CancelJob(const sdpa::events::CancelJobEvent*);
		virtual void CancelJobAck(const sdpa::events::CancelJobAckEvent*);
		virtual void DeleteJob(const sdpa::events::DeleteJobEvent*);
		virtual void JobFailed(const sdpa::events::JobFailedEvent*);
		virtual void JobFinished(const sdpa::events::JobFinishedEvent*);
		virtual void QueryJobStatus(const sdpa::events::QueryJobStatusEvent*);
		virtual void RetrieveJobResults(const sdpa::events::RetrieveJobResultsEvent*);
		virtual void Dispatch(const sdpa::events::SubmitJobAckEvent*);

		virtual sdpa::status_t getStatus() { return "Undefined"; }

    };
}}

#endif

#ifndef SDPA_JOB_HPP
#define SDPA_JOB_HPP 1

#include <string>
#include <vector>
#include <utility>
#include <sdpa/memory.hpp>
#include <sdpa/types.hpp>

#include <boost/statechart/event_base.hpp>
#include <sdpa/events/RunJobEvent.hpp>
#include <sdpa/events/JobFailedEvent.hpp>
#include <sdpa/events/QueryJobStatusEvent.hpp>
#include <sdpa/events/JobStatusReplyEvent.hpp>
#include <sdpa/events/CancelJobEvent.hpp>
#include <sdpa/events/CancelJobAckEvent.hpp>
#include <sdpa/events/DeleteJobEvent.hpp>
#include <sdpa/events/JobFinishedEvent.hpp>
#include <sdpa/events/ErrorEvent.hpp>
#include <sdpa/events/RetrieveResultsEvent.hpp>


namespace sdpa { namespace daemon {

	class GenericDaemon;
    /**
     * The interface to the generic job description we keep around in all
     * components.
     */
    class Job {
    public:
        typedef sdpa::shared_ptr<Job> ptr_t;

        typedef std::string token_t;
        typedef std::string place_t;
        typedef std::pair<place_t, token_t> value_t;
        typedef std::vector<value_t> data_t;

        static const job_id_t &invalid_job_id();

        virtual const job_id_t & id() const = 0;
        virtual const job_id_t & parent() const = 0;

        virtual const job_desc_t & description() const = 0;

        virtual const data_t & input() const = 0;
        virtual const data_t & output() const = 0;

        virtual void add_input(const value_t & value) = 0;
        virtual void add_output(const value_t & value) = 0;

        virtual void add_subjob(const ptr_t & job) = 0;
        virtual ptr_t get_subjob(const job_id_t & id) = 0;

        virtual bool is_marked_for_deletion() = 0;
        virtual void process_event( const boost::statechart::event_base & e)=0;

        //transitions
		virtual void CancelJob(const sdpa::events::CancelJobEvent& event)=0;
		virtual void CancelJobAck(const sdpa::events::CancelJobAckEvent& event)=0;
		virtual void DeleteJob(const sdpa::events::DeleteJobEvent& event)=0;
		virtual void JobFailed(const sdpa::events::JobFailedEvent& event)=0;
		virtual void JobFinished(const sdpa::events::JobFinishedEvent& event)=0;
		virtual void QueryJobStatus(const sdpa::events::QueryJobStatusEvent& event)=0;
		virtual void RetrieveResults(const sdpa::events::RetrieveResultsEvent& event)=0;
		virtual void RunJob(const sdpa::events::RunJobEvent& event)=0;

    };
}}

#endif

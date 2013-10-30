/*
 * =====================================================================================
 *
 *       Filename:  JobEvent.hpp
 *
 *    Description:  JobEvent
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
#ifndef SDPA_JOB_EVENT_HPP
#define SDPA_JOB_EVENT_HPP 1

#include <string>

#include <sdpa/memory.hpp>
#include <sdpa/types.hpp>
#include <sdpa/events/SDPAEvent.hpp>

namespace sdpa {
namespace events {
    class JobEvent : public sdpa::events::SDPAEvent {
    public:
        typedef sdpa::shared_ptr<JobEvent> Ptr;

        JobEvent(const address_t &a_from, const address_t &a_to, const sdpa::job_id_t &a_job_id)
          : SDPAEvent(a_from, a_to)
          , job_id_(a_job_id)
        {
        }

        JobEvent(const address_t &a_from, const address_t &a_to, const sdpa::job_id_t &a_job_id, const message_id_type &mid)
          : SDPAEvent(a_from, a_to, mid)
          , job_id_(a_job_id)
        {
        }

        const sdpa::job_id_t & job_id() const { return job_id_; }
        sdpa::job_id_t & job_id() { return job_id_; }

        virtual std::string str() const = 0;

        int priority() const { return 1; }

        virtual void handleBy(EventHandler *handler) = 0;
    private:
        sdpa::job_id_t job_id_;
    };
}}

#endif // SDPA_JOB_EVENT_HPP

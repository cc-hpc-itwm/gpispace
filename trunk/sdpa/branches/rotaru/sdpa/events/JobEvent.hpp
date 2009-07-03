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

        JobEvent(const address_t &from, const address_t &to, const sdpa::job_id_t &job_id = sdpa::job_id_t())
          : SDPAEvent(from, to), job_id_(job_id) {}
        ~JobEvent() {}

        const sdpa::job_id_t & job_id() const { return job_id_; }

        virtual std::string str() const = 0;
    private:
        sdpa::job_id_t job_id_;
    };
}}

#endif // SDPA_JOB_EVENT_HPP

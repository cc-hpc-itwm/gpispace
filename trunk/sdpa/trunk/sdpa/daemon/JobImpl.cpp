#include "JobImpl.hpp"

namespace sdpa {
    JobImpl::JobImpl(const Job::job_id_t &id,
                     const Job::job_desc_t &desc,
                     const Job::job_id_t &parent)
        : id_(id), desc_(desc), parent_(parent)
    {}

    JobImpl::~JobImpl() throw () { }

    const Job::job_id_t & JobImpl::id() const {
        return id_;
    }

    const Job::job_id_t & JobImpl::parent() const {
        return parent_;
    }

    const Job::job_desc_t & JobImpl::description() const {
        return desc_;
    }

    const Job::data_t & JobImpl::input() const {
        return input_;
    }
    const Job::data_t & JobImpl::output() const {
        return output_;
    }

    void JobImpl::add_input(const Job::value_t & v) {
        input_.push_back(v);
    }

    void JobImpl::add_output(const Job::value_t & v) {
        output_.push_back(v);
    }

    void JobImpl::add_subjob(const Job::ptr_t & job) {
        subjobs_.insert(std::make_pair(job->id(), job));
    }

    Job::ptr_t JobImpl::get_subjob(const job_id_t & jid) {
        return subjobs_[jid];
    }
}


#pragma once

#include <gspc/comm/worker/scheduler/Server.hpp>
#include <gspc/job/FinishReason.hpp>
#include <gspc/job/ID.hpp>

#include <util-generic/scoped_boost_asio_io_service_with_threads.hpp>

namespace gspc
{
  namespace interface
  {
    class Scheduler
    {
    public:
      template<typename Derived>
        Scheduler (Derived*);
      virtual ~Scheduler() = default;

      // called by user
      virtual void wait() = 0;
      virtual void stop() = 0;

      //! \todo Do we require the called to also know they have to call
      //! the workflow engine or should we automatically query that here
      //! as well? Effectively this is we.status().extracted_tasks +
      //! their state.
      //      virtual std::unordered_map<job::Description, job::State> status() const;

      // called by worker
      virtual void finished (job::ID, job::FinishReason) = 0;

    protected:
      fhg::util::scoped_boost_asio_io_service_with_threads
        _io_service_for_workers {1};
      comm::worker::scheduler::Server const _comm_server_for_worker;
    };
  }
}

#include <gspc/interface/Scheduler.ipp>

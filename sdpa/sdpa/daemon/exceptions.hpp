#ifndef SDPA_DAEMON_EXCEPTIONS_HPP
#define SDPA_DAEMON_EXCEPTIONS_HPP 1

#include <sdpa/capability.hpp>

#include <stdexcept>

namespace sdpa {
namespace daemon {

	class NoWorkflowEngine : public std::runtime_error {
	public:
		NoWorkflowEngine()
			: std::runtime_error("No workflow engine!") {}

		virtual ~NoWorkflowEngine() throw() {}
	};

	class JobException : public std::runtime_error {
	public:
		JobException(const std::string &reason, const sdpa::job_id_t& job_id)
			: std::runtime_error(reason), job_id_(job_id) {}

		virtual ~JobException() throw() {}
		const sdpa::job_id_t& job_id() const { return job_id_; }

		private:
		sdpa::job_id_t job_id_;
	};

	class WorkerException : public std::runtime_error {
	public:
		WorkerException(const std::string &reason, const sdpa::worker_id_t& worker_id)
			: std::runtime_error(reason), worker_id_(worker_id){}

		virtual ~WorkerException() throw() {}
		const sdpa::worker_id_t& worker_id() const { return worker_id_; }

		private:
		sdpa::worker_id_t worker_id_;
	};

	class JobNotFoundException : public JobException {
		public:
		JobNotFoundException( const sdpa::job_id_t& job_id) : JobException("Job not found!", job_id) {}
		virtual ~JobNotFoundException() throw() {}
	};

	class NoWorkerFoundException : public std::runtime_error  {
		public:
			NoWorkerFoundException()
			: std::runtime_error("No worker found!") {}
		virtual ~NoWorkerFoundException() throw() {}
	};


	class WorkerNotFoundException : public WorkerException {
		public:
		WorkerNotFoundException( const sdpa::worker_id_t& worker_id)
			: WorkerException("Worker not found!", worker_id) {}
		virtual ~WorkerNotFoundException() throw() {}
	};

	class WorkerAlreadyExistException : public WorkerException {
		public:
		WorkerAlreadyExistException( const sdpa::worker_id_t& worker_id, const sdpa::worker_id_t& agent_uuid )
			: WorkerException("A worker with either the same name or the same uuid has already registerd!", worker_id),
			  agent_uuid_(agent_uuid) {}
		virtual ~WorkerAlreadyExistException() throw() {}

		const sdpa::worker_id_t& agent_uuid() { return agent_uuid_; }
		private:
		sdpa::worker_id_t agent_uuid_;
	};

	class JobNotAddedException : public JobException {
		public:
		JobNotAddedException( const sdpa::job_id_t& job_id) : JobException("Job not added!", job_id) {}
		virtual ~JobNotAddedException() throw() {}
	};

	class JobNotDeletedException : public JobException {
		public:
		JobNotDeletedException( const sdpa::job_id_t& job_id) : JobException("Job not deleted!", job_id) {}
		virtual ~JobNotDeletedException() throw() {}
	};

	class NoJobRequirements : public JobException {
		public:
		NoJobRequirements( const sdpa::job_id_t& job_id) : JobException("No preferences set for this job!", job_id) {}
		virtual ~NoJobRequirements() throw() {}
	};

	class JobNotAssignedException : public JobException {
		public:
		JobNotAssignedException( const sdpa::job_id_t& job_id) : JobException("Job not assigned yet to any worker!", job_id) {}
		virtual ~JobNotAssignedException() throw() {}
	};
}}

#endif

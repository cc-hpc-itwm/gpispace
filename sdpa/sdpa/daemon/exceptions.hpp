#ifndef SDPA_DAEMON_EXCEPTIONS_HPP
#define SDPA_DAEMON_EXCEPTIONS_HPP 1

#include <sdpa/capability.hpp>

#include <stdexcept>

namespace sdpa {
namespace daemon {

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

	class JobNotDeletedException : public JobException {
		public:
		JobNotDeletedException( const sdpa::job_id_t& job_id) : JobException("Job not deleted!", job_id) {}
		virtual ~JobNotDeletedException() throw() {}
	};
}}

#endif

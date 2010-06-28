#ifndef SDPA_DAEMON_EXCEPTIONS_HPP
#define SDPA_DAEMON_EXCEPTIONS_HPP 1

#include <sdpa/SDPAException.hpp>

namespace sdpa {
namespace daemon {
	class JobException : public sdpa::SDPAException {
	public:
		JobException(const std::string &reason, const sdpa::job_id_t& job_id)
			: sdpa::SDPAException(reason), job_id_(job_id) {}

		virtual ~JobException() throw() {}
		const sdpa::job_id_t& job_id() const { return job_id_; }

		private:
		sdpa::job_id_t job_id_;
	};

	class WorkerException : public sdpa::SDPAException {
	public:
		WorkerException(const std::string &reason, const sdpa::worker_id_t& worker_id, const unsigned int rank = 0 )
			: sdpa::SDPAException(reason), worker_id_(worker_id), rank_(rank) {}

		virtual ~WorkerException() throw() {}
		const sdpa::worker_id_t& worker_id() const { return worker_id_; }
		const unsigned int& rank() const { return rank_; }

		private:
		sdpa::worker_id_t worker_id_;
		unsigned int rank_;
	};

	class JobNotFoundException : public JobException {
		public:
		JobNotFoundException( const sdpa::job_id_t& job_id) : JobException("Job not found!", job_id) {}
		virtual ~JobNotFoundException() throw() {}
	};

	class NoWorkerFoundException : public sdpa::SDPAException  {
		public:
			NoWorkerFoundException()
			: sdpa::SDPAException("No worker found!") {}
		virtual ~NoWorkerFoundException() throw() {}
	};


	class WorkerNotFoundException : public WorkerException {
		public:
		WorkerNotFoundException( const sdpa::worker_id_t& worker_id)
			: WorkerException("Worker not found!", worker_id) {}
		virtual ~WorkerNotFoundException() throw() {}
	};

	class NoJobScheduledException : public WorkerException {
		public:
			NoJobScheduledException( const sdpa::worker_id_t& worker_id)
			: WorkerException("No job was scheduled!", worker_id) {}
		virtual ~NoJobScheduledException() throw() {}
	};

	class WorkerAlreadyExistException : public WorkerException {
		public:
		WorkerAlreadyExistException( const sdpa::worker_id_t& worker_id, const unsigned int rank)
			: WorkerException("A worker with either the same name or the same rank already registerd!", worker_id,rank) {}
		virtual ~WorkerAlreadyExistException() throw() {}
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

	class JobNotMarkedException : public JobException {
		public:
		JobNotMarkedException( const sdpa::job_id_t& job_id) : JobException("Job not marked for deletion!", job_id) {}
		virtual ~JobNotMarkedException() throw() {}
	};

	class NoJobPreferences : public JobException {
		public:
		NoJobPreferences( const sdpa::job_id_t& job_id) : JobException("No preferences set for this job!", job_id) {}
		virtual ~NoJobPreferences() throw() {}
	};
}}

#endif

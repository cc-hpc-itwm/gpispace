#ifndef SDPA_DAEMON_EXCEPTIONS_HPP
#define SDPA_DAEMON_EXCEPTIONS_HPP 1

#include <sdpa/SDPAException.hpp>
#include <sdpa/wf/types.hpp>

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
		WorkerException(const std::string &reason, const sdpa::worker_id_t& worker_id)
			: sdpa::SDPAException(reason), worker_id_(worker_id) {}

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

	class WorkerNotFoundException : public WorkerException {
		public:
		WorkerNotFoundException( const sdpa::worker_id_t& worker_id)
			: WorkerException("Worker not found!", worker_id) {}
		virtual ~WorkerNotFoundException() throw() {}
	};

	class NoJobScheduledException : public WorkerException {
		public:
			NoJobScheduledException( const sdpa::worker_id_t& worker_id)
			: WorkerException("Worker not found!", worker_id) {}
		virtual ~NoJobScheduledException() throw() {}
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

	class WorkflowException : public sdpa::SDPAException {
		public:
			WorkflowException(const std::string &reason, const sdpa::wf::workflow_id_t& workflow_id)
			: sdpa::SDPAException(reason), workflow_id_(workflow_id) {}

		virtual ~WorkflowException() throw() {}
		const sdpa::wf::workflow_id_t& workflow_id() const { return workflow_id_; }

		private:
		sdpa::wf::workflow_id_t workflow_id_;
	};

	class NoSuchWorkflowException: public WorkflowException {
		public:
		NoSuchWorkflowException( const sdpa::wf::workflow_id_t& workflow_id)
			: WorkflowException("No such workflow!", workflow_id) {}
		virtual ~NoSuchWorkflowException() throw() {}
	};

	class ActivityException : public sdpa::SDPAException {
		public:
		ActivityException( const std::string &reason, const sdpa::wf::activity_id_t& activity_id )
			: sdpa::SDPAException(reason), activity_id_(activity_id) {}

		virtual ~ActivityException() throw() {}
		const sdpa::wf::activity_id_t& activity_id() const { return activity_id_; }

		private:
		sdpa::wf::activity_id_t activity_id_;
	};

	class NoSuchActivityException: public ActivityException {
		public:
			NoSuchActivityException( const sdpa::wf::activity_id_t& activity_id)
			: ActivityException("No such activity!", activity_id) {}
		virtual ~NoSuchActivityException() throw() {}

	};

}}

#endif

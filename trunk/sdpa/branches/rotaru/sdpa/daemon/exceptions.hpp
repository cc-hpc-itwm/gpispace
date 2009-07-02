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

	class JobNotFoundException : public JobException {
		public:
			JobNotFoundException( const sdpa::job_id_t& job_id) : JobException("Job not found!", job_id) {}
		virtual ~JobNotFoundException() throw() {}
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
}}

#endif

#ifndef SDPA_DAEMON_EXCEPTIONS_HPP
#define SDPA_DAEMON_EXCEPTIONS_HPP 1

#include <sdpa/capability.hpp>

#include <stdexcept>

namespace sdpa {
namespace daemon {
	class JobNotFoundException : public std::runtime_error {
		public:
		JobNotFoundException() : std::runtime_error ("job not found") {}
		virtual ~JobNotFoundException() throw() {}
	};

	class WorkerNotFoundException : public std::runtime_error {
		public:
		WorkerNotFoundException() : std::runtime_error ("worker not found") {}
		virtual ~WorkerNotFoundException() throw() {}
	};

	class JobNotDeletedException : public std::runtime_error {
		public:
		JobNotDeletedException() : std::runtime_error ("job not deleted") {}
		virtual ~JobNotDeletedException() throw() {}
	};
}}

#endif

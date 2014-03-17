#ifndef SDPA_DAEMON_EXCEPTIONS_HPP
#define SDPA_DAEMON_EXCEPTIONS_HPP 1

#include <stdexcept>

namespace sdpa {
namespace daemon {
	class WorkerNotFoundException : public std::runtime_error {
		public:
		WorkerNotFoundException() : std::runtime_error ("worker not found") {}
		virtual ~WorkerNotFoundException() throw() {}
	};
}}

#endif

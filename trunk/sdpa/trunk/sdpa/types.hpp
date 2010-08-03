#ifndef SDPA_TYPES_HPP
#define SDPA_TYPES_HPP 1

#include <string>
#include <sdpa/JobId.hpp>

namespace sdpa {
	typedef sdpa::JobId job_id_t;
	typedef std::string job_desc_t;
	typedef std::string location_t;
	typedef std::string worker_id_t;
	typedef std::string status_t;
	typedef std::string job_result_t;
}

#endif

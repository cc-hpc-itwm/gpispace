#ifndef SDPA_TYPES_HPP
#define SDPA_TYPES_HPP 1

#include <string>
#include <sdpa/JobId.hpp>
#include <vector>

const int MAX_CAPACITY = 10000;

namespace sdpa {
	typedef sdpa::JobId job_id_t;
	typedef std::string job_desc_t;
	typedef std::string location_t;
	typedef std::string worker_id_t;
	typedef std::string status_t;
	typedef std::string job_result_t;
	typedef std::vector<std::string> master_list_t;
}

#endif

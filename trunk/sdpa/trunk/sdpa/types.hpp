#ifndef SDPA_TYPES_HPP
#define SDPA_TYPES_HPP 1

#include <string>
#include <sdpa/JobId.hpp>
#include <vector>
#include <set>
#include <list>

namespace sdpa {
	typedef sdpa::JobId job_id_t;
	typedef std::string job_desc_t;
	typedef std::string location_t;
	typedef std::string worker_id_t;
	typedef std::string status_t;
	typedef std::string job_result_t;
	typedef std::list<sdpa::worker_id_t> worker_id_list_t;
	typedef std::vector<std::string> master_list_t;
	typedef std::pair<std::string,std::string> capability_t;


	struct comp_capabilities
	{
	  bool operator()( capability_t const& a, capability_t const& b)
	  {
		  return a.first < b.first; // compare the uuids
	  }
	};

	typedef std::set<capability_t, comp_capabilities > capabilities_set_t;
}

#endif

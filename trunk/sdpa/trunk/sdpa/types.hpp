#ifndef SDPA_TYPES_HPP
#define SDPA_TYPES_HPP 1

#include <iostream>
#include <string>
#include <list>
#include <map>
#include <set>

#include <sdpa/JobId.hpp>

namespace sdpa {
	typedef sdpa::JobId job_id_t;
	typedef std::string job_desc_t;
	typedef std::string location_t;
	typedef std::string worker_id_t;
	typedef std::string status_t;
	typedef std::string job_result_t;
	typedef std::list<int> list_of_ranks;
	typedef std::set<int> set_of_ranks;

	struct preferences_t
	{
		bool mandatory(){ return mandatory_; }
		const list_of_ranks& ranks() { return ranks_; }
		const set_of_ranks& excluded() { return excluded_ranks_; }
	  private:
		bool mandatory_;
		list_of_ranks ranks_;
		set_of_ranks  excluded_ranks_;
	};

	typedef std::map<sdpa::JobId, sdpa::preferences_t> preferences_map_t;
}

#endif

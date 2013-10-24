#ifndef SDPA_TYPES_HPP
#define SDPA_TYPES_HPP 1

#include <string>
#include <sdpa/JobId.hpp>
#include <vector>
#include <set>
#include <list>
#include <map>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <iostream>
#include <boost/foreach.hpp>

namespace sdpa {
	typedef sdpa::JobId job_id_t;
	typedef std::list<sdpa::JobId> job_id_list_t;
	typedef std::string job_desc_t;
	typedef std::string location_t;
	typedef std::string worker_id_t;
	typedef worker_id_t agent_id_t;
	typedef std::string status_t;
	typedef std::string job_result_t;
	typedef std::list<sdpa::worker_id_t> worker_id_list_t;
	typedef worker_id_list_t agent_id_list_t;
	typedef std::map<agent_id_t, job_id_list_t> subscriber_map_t;
	typedef std::pair<worker_id_t, job_id_t> worker_job_pair_t;
	typedef std::list<worker_job_pair_t> cancellation_list_t;

	typedef std::list<std::pair<sdpa::worker_id_t, int> > list_match_workers_t;

	typedef std::map<sdpa::worker_id_t, int> map_degs_t;

	class MasterInfo
	{
	public:
		MasterInfo(const std::string& name  = "", bool registered = false )
		: name_(name)
		, registered_(registered)
		, nConsecNetFailCnt_(0)
		, nConsecRegAttempts_(0)
		{}

		std::string name() const { return name_; }
		bool is_registered() const { return registered_; }
		void set_registered(bool b) { registered_ = b; }

		unsigned int getConsecNetFailCnt() { return nConsecNetFailCnt_;}
		void incConsecNetFailCnt() { nConsecNetFailCnt_++;}
		void resetConsecNetFailCnt() { nConsecNetFailCnt_=0; }

		unsigned int getConsecRegAttempts() { return nConsecRegAttempts_;}
		void incConsecRegAttempts() { nConsecRegAttempts_++;}
		void resetConsecRegAttempts() { nConsecRegAttempts_=0; }

		template <class Archive>
		void serialize(Archive& ar, const unsigned int)
		{
			ar & name_;
		}
	private:
		std::string name_;
		bool registered_;
		unsigned int nConsecNetFailCnt_;
		unsigned int nConsecRegAttempts_;
	};

	typedef std::vector<MasterInfo> master_info_list_t;
}

inline std::ostream& operator<<(std::ostream& os, sdpa::worker_id_list_t& worker_list)
{
	os<<"(";
	for(sdpa::worker_id_list_t::iterator it=worker_list.begin(); it!=worker_list.end(); it++)
	{
		os<<*it;
		if( boost::next(it) != worker_list.end() )
			os<<",";
		else
			os<<")";
	}

	return os;
}

#endif

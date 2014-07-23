#ifndef SDPA_TYPES_HPP
#define SDPA_TYPES_HPP 1

#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <iostream>
#include <we/type/value.hpp>
#include <sdpa/job_states.hpp>

#include <iterator>

namespace sdpa {
	typedef std::string job_id_t;
	typedef std::list<job_id_t> job_id_list_t;
	typedef std::string job_desc_t;
	typedef std::string location_t;
	typedef std::string worker_id_t;
	typedef worker_id_t agent_id_t;
	typedef std::string job_result_t;
	typedef std::list<sdpa::worker_id_t> worker_id_list_t;
	typedef worker_id_list_t agent_id_list_t;
	typedef std::pair<worker_id_t, job_id_t> worker_job_pair_t;

  typedef std::list<std::pair<sdpa::worker_id_t, int>> list_match_workers_t;

  class worker_id_host_info_t
  {
  public:
    worker_id_host_info_t ( const worker_id_t& worker_id
                          , const std::string& worker_host
                          )
      : worker_id_ (worker_id)
      , worker_host_ (worker_host)
    {}

    const worker_id_t& worker_id() const {return worker_id_;}
    const worker_id_t& worker_host() const {return worker_host_;}

  private:
    worker_id_t worker_id_;
    std::string worker_host_;
  };

  typedef std::multimap<int, sdpa::worker_id_t, std::greater<int>> mmap_match_deg_worker_id_t;

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

  private:
    std::string name_;
    bool registered_;
    unsigned int nConsecNetFailCnt_;
    unsigned int nConsecRegAttempts_;
  };

  typedef std::vector<MasterInfo> master_info_list_t;
  struct discovery_info_t;

  typedef std::set<discovery_info_t> discovery_info_set_t;
  struct discovery_info_t
  {
    discovery_info_t () = default;
    discovery_info_t (job_id_t job_id
                     , boost::optional<sdpa::status::code> state
                     , discovery_info_set_t children
                    )
       : _job_id (job_id)
       , _state (state)
       , _children (children)
     {}

    const job_id_t& job_id() const { return _job_id; }
    const boost::optional<sdpa::status::code> state() const { return _state; }
    const discovery_info_set_t children() const { return _children; }
    void add_child_info(const discovery_info_t& child_info) { _children.insert(child_info); }

    template<class Archive>
    void serialize(Archive &ar, const unsigned int)
    {
      ar & _job_id;
      ar & _state;
      ar & _children;
    }

    bool operator<( const discovery_info_t& other ) const
    {
      return _job_id < other.job_id();
    }

  private:
    job_id_t _job_id;
    boost::optional<sdpa::status::code> _state;
    discovery_info_set_t _children;
  };
}

inline std::ostream& operator<<(std::ostream& os, const sdpa::worker_id_list_t& worker_list)
{
  os<<"(";
  for(sdpa::worker_id_list_t::const_iterator it=worker_list.begin(); it!=worker_list.end(); it++)
  {
      os<<*it;
      if( std::next(it) != worker_list.end() )
        os<<",";
      else
        os<<")";
  }

 return os;
}

#endif

#pragma once

#include <fhgcom/peer_info.hpp>

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
	typedef std::string job_result_t;
	typedef std::list<sdpa::worker_id_t> worker_id_list_t;
	typedef worker_id_list_t agent_id_list_t;
	typedef std::pair<worker_id_t, job_id_t> worker_job_pair_t;

  typedef std::list<std::pair<sdpa::worker_id_t, int>> list_match_workers_t;

  using name_host_port_tuple
    = std::tuple<std::string, fhg::com::host_t, fhg::com::port_t>;

  namespace daemon
  {
    class GenericDaemon;
  }

  //! \note Defined in GenericDaemon.cpp
  struct opaque_job_master_t
  {
    opaque_job_master_t() = delete;
    opaque_job_master_t (opaque_job_master_t const&) = delete;
    opaque_job_master_t& operator= (opaque_job_master_t const&) = delete;
    opaque_job_master_t (opaque_job_master_t&&);
    opaque_job_master_t& operator= (opaque_job_master_t&&) = delete;
    ~opaque_job_master_t();

  private:
    opaque_job_master_t (const void*);
    struct implementation;
    friend class sdpa::daemon::GenericDaemon;
    implementation* _;
    implementation* operator->() const { return _; }
  };

  class worker_id_host_info_t
  {
  public:
    worker_id_host_info_t ( const worker_id_t& worker_id
                          , const std::string& worker_host
                          , unsigned long shared_memory_size
                          , const double& last_time_served
                          )
      : worker_id_ (worker_id)
      , worker_host_ (worker_host)
      , shared_memory_size_ (shared_memory_size)
      , last_time_served_ (last_time_served)
    {}

    const worker_id_t& worker_id() const {return worker_id_;}
    const std::string& worker_host() const {return worker_host_;}
    double last_time_served() const { return  last_time_served_;}
    unsigned long shared_memory_size() const {return shared_memory_size_;}

  private:
    worker_id_t worker_id_;
    std::string worker_host_;
    unsigned long shared_memory_size_;
    double last_time_served_;
  };

  typedef std::multimap<int, worker_id_host_info_t, std::greater<int>> mmap_match_deg_worker_id_t;

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

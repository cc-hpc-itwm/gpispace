#pragma once

#include <sdpa/job_states.hpp>

#include <fhgcom/peer.hpp>
#include <fhgcom/peer_info.hpp>

#include <boost/optional.hpp>

#include <forward_list>
#include <map>
#include <set>
#include <string>

namespace sdpa {
	typedef std::string job_id_t;
	typedef std::string worker_id_t;

  namespace daemon
  {
    class GenericDaemon;
  }

  struct master_network_info
  {
    master_network_info (std::string const& host_, std::string const& port_)
      : host (host_)
      , port (port_)
      , address (boost::none)
    {}

    fhg::com::host_t host;
    fhg::com::port_t port;
    boost::optional<fhg::com::p2p::address_t> address;
  };
  using master_info_t = std::forward_list<master_network_info>;

  class worker_id_host_info_t
  {
  public:
    worker_id_host_info_t ( const worker_id_t& worker_id
                          , const std::string& worker_host
                          , unsigned long shared_memory_size
                          , const double& last_time_idle
                          )
      : worker_id_ (worker_id)
      , worker_host_ (worker_host)
      , shared_memory_size_ (shared_memory_size)
      , _last_time_idle (last_time_idle)
    {}

    const worker_id_t& worker_id() const {return worker_id_;}
    const std::string& worker_host() const {return worker_host_;}
    double last_time_idle() const {return _last_time_idle;}
    unsigned long shared_memory_size() const {return shared_memory_size_;}

  private:
    worker_id_t worker_id_;
    std::string worker_host_;
    unsigned long shared_memory_size_;
    double _last_time_idle;
  };

  typedef std::multimap<double, worker_id_host_info_t, std::greater<double>> mmap_match_deg_worker_id_t;

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

#pragma once

#include <sdpa/job_states.hpp>

#include <fhgcom/peer.hpp>
#include <fhgcom/peer_info.hpp>

#include <vmem/types.hpp>

#include <boost/functional/hash.hpp>
#include <boost/optional.hpp>

#include <forward_list>
#include <map>
#include <set>
#include <string>

namespace sdpa {
	typedef std::string job_id_t;
	typedef std::string worker_id_t;
	typedef std::string cache_id_t;

  using name_host_port_tuple
    = std::tuple<std::string, fhg::com::host_t, fhg::com::port_t>;

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
                          , boost::optional<intertwine::vmem::rank_t> vmem_rank
                          , boost::optional<intertwine::vmem::size_t> vmem_cache_size
                          , const double& last_time_idle
                          )
      : worker_id_ (worker_id)
      , _vmem_rank (vmem_rank)
      , _vmem_cache_size (vmem_cache_size)
      , _last_time_idle (last_time_idle)
    {}

    const worker_id_t& worker_id() const {return worker_id_;}
    double last_time_idle() const {return _last_time_idle;}
    boost::optional<intertwine::vmem::rank_t> vmem_rank() const { return _vmem_rank; }
    boost::optional<intertwine::vmem::size_t> vmem_cache_size() const { return _vmem_cache_size; }

  private:
    worker_id_t worker_id_;
    boost::optional<intertwine::vmem::rank_t> _vmem_rank;
    boost::optional<intertwine::vmem::size_t> _vmem_cache_size;
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

  class cache_info_t
  {
  public:
    cache_info_t() = delete;
    cache_info_t (std::size_t size)
      : _size (size)
      , _available (size)
      , _ref_count (0)
    {}
    cache_info_t (cache_info_t const&) = delete;
    cache_info_t (cache_info_t&&) = delete;
    cache_info_t& operator= (cache_info_t const&) = delete;
    cache_info_t& operator= (cache_info_t&&) = delete;

    std::size_t size() const { return _size; }
    std::size_t available() const { return _available; }
    void shrink (std::size_t const& amount) { _available -= amount; }
    void grow (std::size_t const& amount) { _available += amount; }
    int inc_ref_count() { return ++_ref_count; }
    int dec_ref_count() { return --_ref_count; }

  private:
    std::size_t _size;
    std::size_t _available;
    int _ref_count;
  };

  typedef std::pair<intertwine::vmem::rank_t, intertwine::vmem::cache_id_t> cache_info_key_t;
  struct cache_info_key_hasher
  {
    std::size_t operator() (cache_info_key_t const& key) const
    {
      std::size_t seed = 0;
      boost::hash_combine (seed, boost::hash_value (key.first));
      boost::hash_combine (seed, key.second.hash_value());
      return seed;
    }
  };
}

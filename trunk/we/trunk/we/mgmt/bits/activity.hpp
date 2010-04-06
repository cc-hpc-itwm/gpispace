/*
 * =====================================================================================
 *
 *       Filename:  activity.hpp
 *
 *    Description:  implements a generic activity descriptor
 *
 *        Version:  1.0
 *        Created:  03/25/2010 12:00:47 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Petry (petry), alexander.petry@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#ifndef WE_MGMT_BITS_ACTIVITY_HPP
#define WE_MGMT_BITS_ACTIVITY_HPP 1

#include <vector>
#include <boost/unordered_set.hpp>
#include <boost/bimap.hpp>
#include <boost/bimap/unordered_set_of.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/random.hpp>

namespace we { namespace mgmt { namespace detail {
  template <typename Net>
  struct activity_traits
  {
    typedef Net net_type;
    typedef petri_net::pid_t pid_t;
    typedef petri_net::tid_t tid_t;

    typedef typename net_type::token_type token_type;
//    struct token_on_place_t : std::pair<token_type, pid_t>
//    {
//      token_type const & token() const { return this->first; }
//      token_type & token() { return this->first; }
//      pid_t place() { return this->second; }
//    };
    typedef std::pair<token_type, pid_t> token_on_place_t;
    typedef std::vector<token_on_place_t> input_t;
    typedef std::vector<token_on_place_t> output_t;

//    typedef typename net_type::input_t input_t;
//    typedef typename net_type::output_t output_t;
    typedef typename net_type::output_descr_t output_descr_t;
  };

  template <typename Id, typename ModuleCall, typename Net, typename Expression, typename Traits = activity_traits<Net>, typename Count = unsigned long>
  class activity
  {
    typedef activity<Id, ModuleCall, Net, Expression> this_type;

    typedef Id id_type;
    typedef ModuleCall module_call_type;
    typedef Net net_type;
    typedef Expression expression_type;
    typedef Traits traits_type;
    typedef Count size_type;

    typedef typename traits_type::input_t input_t;
    typedef typename traits_type::output_t output_t;
    typedef typename traits_type::output_descr_t output_descr_t;
    typedef typename traits_type::pid_t pid_t;
    typedef typename traits_type::tid_t tid_t;

    typedef boost::unordered_set<id_type> children_set_t;
    typedef boost::shared_lock<this_type> shared_lock_t;
    typedef boost::unique_lock<this_type> unique_lock_t;

    // handle mapping to and from transition-local places
    typedef boost::bimaps::unordered_set_of<pid_t> pid_collection_t;
    typedef boost::bimap<pid_collection_t, pid_collection_t> pid_map_t;

    enum ActivityType
    {
      MODULE_CALL = 0
    , EXPRESSION
    , NET
    };

    union
    {
      module_call_type *module_call;
      net_type *net;
      expression_type *expression;
    } data;

    struct
    {
      bool internal : 1;

      bool suspended : 1;
      bool cancelling : 1;
      bool cancelled : 1;
      bool failed : 1;
    } flags;

    inline
    bool is_alive() const
    {
      shared_lock_t lock(*this);
      return ( flags.supended || flags.cancelling || flags.cancelled ) == false;
    }

    template <typename IdGen>
    this_type
    extract(IdGen id_gen)
    {
      unique_lock_t lock(*this);

      assert ( flags.internal && type_ == NET );

      typename net_type::activity_t net_activity = data.net->extract_activity_random(engine_);

      throw std::runtime_error("not implemented yet");
    }

    template <typename Output>
    void
    inject(const Output & o)
    {
      unique_lock_t lock(*this);

      if (type_ == NET)
      {
        data.net->inject_activity_result (o);
      }
      else
      {
        output_ = o;
      }

      throw std::runtime_error("not implemented yet");
    }

    bool
    done (void) const
    {
      shared_lock_t lock(*this);
      if (type_ == NET)
      {
        return (extracted_ == injected_) && ( ! has_enabled() );
      }
      else
      {
        return ! output_.empty();
      }
    }

    bool
    has_enabled (void) const
    {
      shared_lock_t lock(*this);
      return ! (this->data.net->enabled_transitions().empty());
    }

    size_t
    num_enabled (void) const
    {
      shared_lock_t lock(*this);
      return (this->data.net->enabled_transitions().size());
    }

    void
    map_place (const pid_t outer, const pid_t inner)
    {
      unique_lock_t lock(*this);
      pid_mapping_.insert(typename pid_map_t::value_type(outer, inner));
    }

    // **********************************
    //
    // Lockable concept implementation
    //
    // **********************************
    void lock()
    {
      mutex_.lock();
    }

    bool try_lock()
    {
      return mutex_.try_lock();
    }

    void unlock()
    {
      mutex_.unlock();
    }

    // timed lockable concept implementation
    bool timed_lock(boost::system_time const& abs_time)
    {
      return mutex_.timed_lock(abs_time);
    }

    template<typename DurationType> bool timed_lock(DurationType const& rel_time)
    {
      return mutex_.timed_lock(rel_time);
    }

    // shared lockable concept implementation
    void lock_shared()
    {
      mutex_.lock_shared();
    }

    bool try_lock_shared()
    {
      return mutex_.try_lock_shared();
    }

    bool timed_lock_shared(boost::system_time const& abs_time)
    {
      return mutex_.timed_lock_shared(abs_time);
    }

    template<typename DurationType> bool timed_lock_shared(DurationType const& rel_time)
    {
      return mutex_.timed_lock_shared(rel_time);
    }

    void unlock_shared()
    {
      mutex_.unlock_shared();
    }

  private:
    inline
    pid_t map_to_local(const pid_t p) const
    {
      const typename pid_map_t::right_map::const_iterator it (pid_mapping_.left.find(p));
      if (it == pid_mapping_.left.end())
        return p;
      else
        return it->second;
    }

    inline
    pid_t map_from_local(const pid_t p) const
    {
      const typename pid_map_t::left_map::const_iterator it (pid_mapping_.right.find(p));
      if (it == pid_mapping_.right.end())
        return p;
      else
        return it->second;
    }

    friend class boost::serialization::access;
    template<class Archive>
    void serialize (Archive & ar, const unsigned int version)
    {
      ar & BOOST_SERIALIZATION_NVP(type_);
      ar & BOOST_SERIALIZATION_NVP(id_);
      ar & BOOST_SERIALIZATION_NVP(children_);
      ar & BOOST_SERIALIZATION_NVP(parent_);
      ar & BOOST_SERIALIZATION_NVP(input_);
      ar & BOOST_SERIALIZATION_NVP(output_);
      ar & BOOST_SERIALIZATION_NVP(output_descr_);
      ar & BOOST_SERIALIZATION_NVP(pid_mapping_);
    }

  private:
    ActivityType type_;
    id_type id_;
    mutable boost::shared_mutex mutex_;
    children_set_t children_;
    id_type parent_;

    input_t input_;
    output_t output_;
    output_descr_t output_descr_;
    pid_map_t pid_mapping_;

    size_type extracted_;
    size_type injected_;
    boost::mt19937 engine_;
  };
}}}

#endif

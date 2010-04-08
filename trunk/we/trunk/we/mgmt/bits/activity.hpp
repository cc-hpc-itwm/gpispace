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

#include <we/mgmt/bits/pid_map_t.hpp>

namespace we { namespace mgmt { namespace detail {
  template <typename Transition>
  struct activity_traits
  {
    typedef petri_net::pid_t pid_t;
    typedef petri_net::tid_t tid_t;
    typedef Transition transition_type;

    typedef typename transition_type::net_type net_type;
    typedef typename transition_type::mod_type mod_type;
    typedef typename transition_type::expr_type expr_type;
    typedef typename transition_type::category_t transition_cat_t;

    typedef typename net_type::token_type token_type;
    typedef std::pair<token_type, pid_t> token_on_place_t;
    typedef std::vector<token_on_place_t> input_t;
    typedef std::vector<token_on_place_t> output_t;
  };

  template <typename Id, typename Transition, typename Traits = activity_traits<Transition> >
  class activity
  {
    typedef activity<Id, Transition, Traits> this_type;

  public:
    typedef Id id_type;
    typedef Transition transition_type;
    typedef Traits traits_type;

    typedef typename traits_type::net_type net_type;
    typedef typename traits_type::mod_type mod_type;
    typedef typename traits_type::expr_type expr_type;

    typedef typename traits_type::token_on_place_t token_on_place_t;
    typedef typename traits_type::input_t input_t;
    typedef typename traits_type::output_t output_t;
    typedef typename traits_type::pid_t pid_t;
    typedef typename traits_type::tid_t tid_t;

    typedef boost::unordered_set<id_type> children_set_t;
    typedef boost::shared_lock<this_type> shared_lock_t;
    typedef boost::unique_lock<this_type> unique_lock_t;

    // handle mapping to and from transition-local places
    typedef traits::pid_map_traits<pid_t> pid_map_traits;
    typedef typename pid_map_traits::type pid_map_t;

    explicit
    activity (const id_type id)
      : id_ (id)
    {
      flags_ = 0;
    }

    template <typename T>
    activity (const id_type id, const T & transition)
      : id_ (id)
      , transition_ (transition)
    {
      flags_ = 0;
    }

    activity (const this_type & other)
      : id_ (other.id_)
      , parent_ (other.parent_)
      , flags_ (other.flags_)
      , children_ (other.children_)
      , transition_ (other.transition_)
      , input_ (other.input_)
      , output_ (other.output_)
    { }

    activity & operator= (const this_type & other)
    {
      if (this != &other)
      {
        id_  = (other.id_);
        parent_ = (other.parent_);
        flags_ = (other.flags_);
        children_ = (other.children_);
        transition_ = (other.transition_);
        input_ = (other.input_);
        output_ = (other.output_);
      }
      return *this;
    }

    template <typename T>
    void assign( T const & t )
    {
      unique_lock_t lock(*this);
      transition_ = t;
    }

    struct flags_t
    {
      bool suspended : 1;
      bool cancelling : 1;
      bool cancelled : 1;
      bool failed : 1;

      flags_t ()
        : suspended(0)
        , cancelling(0)
        , cancelled(0)
        , failed(0)
      { }
    };

    inline
    flags_t const & flags() const
    {
      return flags_;
    }
    inline
    flags_t & flags()
    {
      return flags_;
    }

    inline
    bool is_alive() const
    {
      shared_lock_t lock(const_cast<this_type&>(*this));
      return ( flags_.suspended || flags_.cancelling || flags_.cancelled ) == false;
    }

    inline
    bool is_leaf() const
    {
      shared_lock_t lock(const_cast<this_type&>(*this));
      return children_.empty();
    }

    inline
    id_type id() const
    {
      shared_lock_t lock(const_cast<this_type&>(*this));
      return id_;
    }

    inline
    id_type parent() const
    {
      shared_lock_t lock(const_cast<this_type&>(*this));
      return parent_;
    }

    inline
    const transition_type & transition() const
    {
      return transition_;
    }

    this_type
    extract(id_type id)
    {
      unique_lock_t lock(const_cast<this_type&>(*this));

      typename net_type::activity_t net_activity = transition_.template as<net_type>()->extract_activity_random(engine_);

      this_type act = this->create_activity_from_net_activity (net_activity, id);

      return act;
    }

    // TODO: work here
    template <typename Output>
    void
    inject_results(const Output & o)
    {
      unique_lock_t lock(*this);

      // the passed results are in "local view", we have to map them before we
      // can inject
      output_t mapped_output;
      mapped_output.reserve (o.size());
      map_from_local (o.begin(), o.end(), std::back_inserter(mapped_output));

      if (transition_.is_net())
      {
        transition_. template as<net_type>()->inject_activity_result (mapped_output);
      }
      else
      {
        output_ = mapped_output;
      }
    }

    template <typename Activity>
    void
    child_finished (const Activity & act)
    {
      unique_lock_t lock(*this);

      output_t mapped_output;
      mapped_output.reserve (act.output().size());
      act.map_from_local (act.output().begin(), act.output().end(), std::back_inserter(mapped_output));

      transition_. template as<net_type>()->inject_activity_result (mapped_output);

      unlink_child (act);
    }

    void prepare_input()
    {
      unique_lock_t lock(*this);
      if (transition_.is_net())
      {
        for (typename input_t::const_iterator it = input_.begin(); it != input_.end(); ++it)
        {
          typename pid_map_traits::result_type map_result =
            pid_map_traits::map_to( transition_.i_mapping, it->second );
          // should always be safe!
          if (map_result.second)
          {
            transition_.template as<net_type>()->put_token( map_result.first, it->first );
            std::cerr << "\tmapped token " << it->first << " on outer place " << it->second << " to inner " << map_result.first << std::endl;
          }
        }
      }
      else
      {
        // nothing to do (TODO?)
      }
    }

    template <typename Context>
    void execute ( Context & )
    {
      if (transition_.is_expr())
      {
        std::cerr << "executing expression" << std::endl;
      }
    }

    bool
    done (void) const
    {
      shared_lock_t lock(const_cast<this_type&>(*this));
      if (transition_.is_net())
      {
        return (this->children_.empty()) && ( ! has_enabled() );
      }
      else
      {
        return ! output_.empty();
      }
    }

    bool
    has_enabled (void) const
    {
      shared_lock_t lock(const_cast<this_type&>(*this));
      if (transition_.is_net())
      {
        return ! (transition_.template as<net_type>()->enabled_transitions().empty());
      }
      else
      {
        return false;
      }
    }

    const input_t & input() const
    {
      shared_lock_t lock(const_cast<this_type&>(*this));
      return input_;
    }
    input_t & input()
    {
      shared_lock_t lock(const_cast<this_type&>(*this));
      return input_;
    }

    const output_t & output() const
    {
      shared_lock_t lock(const_cast<this_type&>(*this));
      return output_;
    }

    template <typename Output>
    void
    set_output ( const Output & o )
    {
      unique_lock_t lock(*this);
      output_ = o;
    }

    size_t
    num_enabled (void) const
    {
      shared_lock_t lock(const_cast<this_type&>(*this));
      if (transition_.is_net())
        return (transition_.template as<net_type>()->enabled_transitions().size());
      else
        return 0;
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
    template <typename Iter, typename OutputIterator>
    void map_from_local(Iter begin, Iter end, OutputIterator dst) const
    {
      for (Iter it (begin); it != end; ++it)
      {
        typename pid_map_traits::result_type map_result = 
          pid_map_traits::map_from( transition_.o_mapping, it->second );
        if (map_result.second) // mapped
        {
          *dst++ = token_on_place_t (it->first, map_result.first);
        }
      }
    }

    template <typename Iter, typename OutputIterator>
    void map_to_local(Iter begin, Iter end, OutputIterator dst) const
    {
      for (Iter it (begin); it != end; ++it)
      {
        typename pid_map_traits::result_type map_result = 
          pid_map_traits::map_to( transition_.i_mapping, it->second );
        if (map_result.second) // mapped
        {
          *dst++ = token_on_place_t (it->first, map_result.first);
        }
      }
    }

    template <typename NetActivity>
    this_type
    create_activity_from_net_activity (NetActivity const & net_act, const id_type id)
    {
      this_type act(id);
      this->link_with_child(act);
      typename net_type::transition_type trans = transition_.template as<net_type>()->get_transition (net_act.tid);
      act.assign (trans);
      act.input_.reserve (net_act.input.size());

      for (typename net_type::input_t::const_iterator it = net_act.input.begin(); it != net_act.input.end(); ++it)
      {
        typename pid_map_traits::result_type map_result = 
          pid_map_traits::map_to( trans.i_mapping, it->second.first );
        if (map_result.second) // mapped
        {
          act.input_.push_back ( token_on_place_t(it->first, map_result.first) );
        }
      }

      return act;
    }

    template <typename Activity>
    void link_with_child(Activity & act)
    {
      act.parent_ = this->id_;
      this->children_.insert( act.id_ );
    }

    template <typename Activity>
    void unlink_child(Activity const & act)
    {
      this->children_.erase ( act.id_ );
    }

    friend class boost::serialization::access;
    template<class Archive>
    void serialize (Archive & ar, const unsigned int version)
    {
      ar & BOOST_SERIALIZATION_NVP(id_);
      ar & BOOST_SERIALIZATION_NVP(parent_);
      ar & BOOST_SERIALIZATION_NVP(flags_);
      ar & BOOST_SERIALIZATION_NVP(children_);
      ar & BOOST_SERIALIZATION_NVP(transition_);
      ar & BOOST_SERIALIZATION_NVP(input_);
      ar & BOOST_SERIALIZATION_NVP(output_);
    }

  private:
    id_type id_;
    id_type parent_;
    flags_t flags_;
    mutable boost::shared_mutex mutex_;
    children_set_t children_;

    transition_type transition_;

    input_t input_;
    output_t output_;

    boost::mt19937 engine_;
  };
}}}

#endif

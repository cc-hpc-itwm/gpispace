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
    typedef typename traits_type::transition_cat_t activity_cat_t;

    typedef boost::unordered_set<id_type> children_set_t;
    typedef boost::shared_lock<this_type> shared_lock_t;
    typedef boost::unique_lock<this_type> unique_lock_t;

    // handle mapping to and from transition-local places
    typedef traits::pid_map_traits<pid_t> pid_map_traits;
    typedef typename pid_map_traits::type pid_map_t;

    explicit
    activity (const id_type id)
      : id_ (id)
    { }

    activity (const this_type & other)
      : id_ (other.id_)
      , type_ (other.type_)
      , parent_ (other.parent_)
      , children_ (other.children_)
      , input_ (other.input_)
      , output_ (other.output_)
      , extracted_ (other.extracted_)
      , injected_ (other.injected_)
    { }

    activity & operator= (const this_type & other)
    {
      if (this != &other)
      {
        id_  = (other.id_);
        type_  = (other.type_);
        parent_ = (other.parent_);
        children_ = (other.children_);
        input_ = (other.input_);
        output_ = (other.output_);
        extracted_ = (other.extracted_);
        injected_ = (other.injected_);
      }
      return *this;
    }

    void assign( net_type const & net )
    {
      if (data.ptr)
        clear();
      type_ = transition_type::NET;
      data.net = new net_type (net);
    }

    void assign( expr_type const & expr )
    {
      clear();
      type_ = transition_type::EXPRESSION;
      data.expr = new expr_type (expr);
    }

    void assign( mod_type const & mod )
    {
      clear();
      type_ = transition_type::MOD_CALL;
      data.mod = new mod_type (mod);
    }

    inline
    void clear()
    {
      if (data.ptr)
      {
        switch (type_)
        {
          case transition_type::NET:
            delete data.net;
            break;
          case transition_type::MOD_CALL:
            delete data.mod;
            break;
          case transition_type::EXPRESSION:
            delete data.expr;
            break;
          default:
            assert(false);
        }
      }
      data.ptr = 0;
    }

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
      shared_lock_t lock(const_cast<this_type&>(*this));
//      shared_lock_t lock(mutex_);
      return ( flags.suspended || flags.cancelling || flags.cancelled ) == false;
    }

    inline
    bool is_leaf() const
    {
//      shared_lock_t lock(*this);
//      shared_lock_t lock(mutex_);
      return children_.empty();
    }

    template <typename Category>
    void setType(const Category cat)
    {
      type_ = cat;
    }

    inline
    id_type id() const
    {
      return id_;
    }

    inline
    id_type parent() const
    {
      return parent_;
    }

    template <typename IdGen>
    this_type
    extract(IdGen id_gen)
    {
//      unique_lock_t lock(*this);
//      unique_lock_t lock(mutex_);

      // this function is only valid for net-types!
      assert ( flags.internal && type_ == transition_type::NET );

      typename net_type::activity_t net_activity = data.net->extract_activity_random(engine_);

      this_type act = this->create_activity_from_net_activity (net_activity, id_gen());

      return act;
    }

    // TODO: work here
    template <typename Output>
    void
    inject(const Output & o)
    {
//      unique_lock_t lock(*this);
//      unique_lock_t lock(mutex_);

      // the passed results are in "local view", we have to map them before we
      // can inject
      output_t mapped_output;
      mapped_output.reserve (o.size());
      map_from_local (o.begin(), o.end(), mapped_output.begin());

      if (type_ == transition_type::NET)
      {
        data.net->inject_activity_result (mapped_output);
      }
      else
      {
        output_ = mapped_output;
      }

      throw std::runtime_error("not implemented yet");
    }

    bool
    done (void) const
    {
//      shared_lock_t lock(*this);
      if (type_ == transition_type::NET)
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
//      shared_lock_t lock(*this);
      if (type_ == transition_type::NET)
        return ! (this->data.net->enabled_transitions().empty());
      else
        return false;
    }

    const output_t & output() const
    {
//      shared_lock_t lock(*this);
      return output_;
    }

    size_t
    num_enabled (void) const
    {
//      shared_lock_t lock(*this);
      if (type_ == transition_type::NET)
        return ! (this->data.net->enabled_transitions().empty());
      else
        return 0;
    }

    void
    map_place (const pid_t outer, const pid_t inner)
    {
//      unique_lock_t lock(*this);
//      unique_lock_t lock(mutex_);
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
    template <typename Iter, typename OutputIterator>
    void map_from_local(Iter begin, Iter end, OutputIterator dst)
    {
      for (Iter it (begin); it != end; ++it)
      {
        typename pid_map_traits::result_type map_result = 
          pid_map_traits::map_from( pid_mapping_, it->second );
        if (map_result.second) // mapped
        {
          *dst++ = token_on_place_t (it->first, map_result.first);
        }
      }
    }

    template <typename Iter, typename OutputIterator>
    void map_to_local(Iter begin, Iter end, OutputIterator dst)
    {
      for (Iter it (begin); it != end; ++it)
      {
        typename pid_map_traits::result_type map_result = 
          pid_map_traits::map_to( pid_mapping_, it->second.first );
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
      typename net_type::transition_type trans = this->data.net->get_transition (net_act.tid);
      act.type_ = trans.type;
      act.flags.internal = trans.flags.internal;
      act.pid_mapping_ = trans.mapping;
      act.input_.reserve (net_act.input.size());

      act.map_to_local (net_act.input.begin(), net_act.input.end(), std::back_inserter(act.input_));

      return act;
    }

    // TODO: remove this overload, it's only here until we remove edges from the net
    inline
    token_on_place_t map_to_local(const typename net_type::token_input_t & t_p_via_e) const
    {
      return token_on_place_t (t_p_via_e.first, map_to_local(t_p_via_e.second.first));
    }

    inline
    token_on_place_t map_to_local(const token_on_place_t & top) const
    {
      return token_on_place_t (top.first, map_to_local(top.second));
    }

    inline
    token_on_place_t map_from_local(const token_on_place_t & top) const
    {
      return token_on_place_t (top.first, map_from_local(top.second));
    }

    template <typename Activity>
    void link_with_child(Activity & act)
    {
      act.parent_ = this->id_;
      this->children_.insert( act.id_ );
    }

    template <typename Activity>
    void unlink_child(Activity & act)
    {
      this->children_.erase ( act.id_ );
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
      ar & BOOST_SERIALIZATION_NVP(pid_mapping_);
    }

  private:
    id_type id_;
    activity_cat_t type_;
    id_type parent_;
    mutable boost::shared_mutex mutex_;
    children_set_t children_;

    union
    {
      mod_type *mod;
      net_type *net;
      expr_type *expr;
      void *ptr;
    } data;

    input_t input_;
    output_t output_;
    pid_map_t pid_mapping_;

    size_t extracted_;
    size_t injected_;
    boost::mt19937 engine_;
  };
}}}

#endif

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

#ifndef WE_MGMT_TYPE_ACTIVITY_HPP
#define WE_MGMT_TYPE_ACTIVITY_HPP 1

#include <vector>
#include <boost/unordered_set.hpp>
#include <boost/bimap.hpp>
#include <boost/bimap/unordered_set_of.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/random.hpp>

#include <we/type/id.hpp>
#include <we/mgmt/bits/transition_visitors.hpp>

namespace we { namespace mgmt { namespace type {
  template <typename Transition, typename Token>
  struct activity_traits
  {
    typedef petri_net::pid_t pid_t;
    typedef Transition transition_type;
    typedef Token token_type;

    typedef std::pair<token_type, pid_t> token_on_place_t;
    typedef std::vector<token_on_place_t> input_t;
    typedef std::vector<token_on_place_t> output_t;
  };

  template <typename Transition, typename Token, typename Id = petri_net::pid_t, typename Traits = activity_traits<Transition, Token> >
  class activity_t
  {
    typedef activity_t<Transition, Token, Id, Traits> this_type;

  public:
    typedef Transition transition_type;
    typedef Id id_type;
    typedef Traits traits_type;

    typedef typename traits_type::token_type token_type;
    typedef typename traits_type::token_on_place_t token_on_place_t;
    typedef typename traits_type::input_t input_t;
    typedef typename traits_type::output_t output_t;
    typedef typename traits_type::pid_t pid_t;

    typedef boost::unordered_set<id_type> children_set_t;
    typedef boost::shared_lock<this_type> shared_lock_t;
    typedef boost::unique_lock<this_type> unique_lock_t;

    activity_t ()
      : id_ (petri_net::traits::id_traits<id_type>::invalid())
      , parent_ (petri_net::traits::id_traits<id_type>::invalid())
    { }

    explicit
    activity_t (const id_type id)
      : id_ (id)
      , parent_ (petri_net::traits::id_traits<id_type>::invalid())
    { }

    template <typename T>
    activity_t (const id_type id, const T & transition)
      : id_ (id)
      , parent_ (petri_net::traits::id_traits<id_type>::invalid())
      , transition_ (transition)
    { }

    template <typename T>
    activity_t (const T & transition)
      : id_ (petri_net::traits::id_traits<id_type>::invalid())
      , parent_ (petri_net::traits::id_traits<id_type>::invalid())
      , transition_ (transition)
    { }

    template <typename T>
    activity_t (const id_type id, const T & transition, const id_type parent)
      : id_ (id)
      , parent_ (parent)
      , transition_ (transition)
    { }

    activity_t (const this_type & other)
      : id_ (other.id_)
      , parent_ (other.parent_)
      , flags_ (other.flags_)
      , children_ (other.children_)
      , transition_ (other.transition_)
      , input_ (other.input_)
      , output_ (other.output_)
    { }

    this_type & operator= (const this_type & other)
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

    // TODO remove this?
    template <typename T>
    void assign( T const & t )
    {
      unique_lock_t lock(*this);
      transition_ = t;
    }

    struct flags_t
    {
      bool suspended;
      bool cancelling;
      bool cancelled;
      bool failed;

      flags_t ()
        : suspended(false)
        , cancelling(false)
        , cancelled(false)
        , failed(false)
      { }

    private:
      friend class boost::serialization::access;
      template<class Archive>
      void serialize (Archive & ar, const unsigned int)
      {
        ar & BOOST_SERIALIZATION_NVP(suspended);
        ar & BOOST_SERIALIZATION_NVP(cancelling);
        ar & BOOST_SERIALIZATION_NVP(cancelled);
        ar & BOOST_SERIALIZATION_NVP(failed);
      }
    };

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
    bool is_suspended() const
    {
      shared_lock_t lock(const_cast<this_type&>(*this));
      return flags_.suspended;
    }

    inline
    bool is_cancelling() const
    {
      shared_lock_t lock(const_cast<this_type&>(*this));
      return flags_.cancelling;
    }

    inline
    bool is_cancelled() const
    {
      shared_lock_t lock(const_cast<this_type&>(*this));
      return flags_.cancelled;
    }

    inline
    bool has_failed() const
    {
      shared_lock_t lock(const_cast<this_type&>(*this));
      return flags_.failed;
    }

    inline
    id_type id() const
    {
      shared_lock_t lock(const_cast<this_type&>(*this));
      return id_;
    }

    inline
    void set_id ( const id_type new_id )
    {
      unique_lock_t lock(*this);
      id_ = new_id;
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

    inline
    transition_type & transition()
    {
      return transition_;
    }

    this_type
    extract(const id_type id)
    {
      unique_lock_t lock(*this);
      we::mgmt::visitor::activity_extractor<this_type, boost::mt19937>
        extract_activity(engine_);
      this_type act = boost::apply_visitor ( extract_activity
                                           , transition_.data()
                                           );
      act.set_id (id);
      this->link_child (act);
      return act;
    }

    void
    inject (this_type & subact)
    {
      unique_lock_t lock(*this);
      we::mgmt::visitor::activity_injector<this_type>
        inject_activity (*this, subact);
      boost::apply_visitor ( inject_activity
                           , transition_.data()
                           , subact.transition_.data()
                           );

      unlink_child (subact);
    }

    template <typename Context>
    void execute ( Context & )
    {
      if (transition ().is_internal ())
      {
        we::mgmt::visitor::internal_executor<this_type> visitor_executor (*this);
        boost::apply_visitor (visitor_executor, transition().data());
      }
    }

    bool
    done (void) const
    {
      // WORK TODO
      shared_lock_t lock(const_cast<this_type&>(*this));
      return (this->children_.empty()) && ( ! has_enabled());
    }

    bool
    has_enabled (void) const
    {
      static const we::mgmt::visitor::has_enabled visitor_has_enabled;

      shared_lock_t lock(const_cast<this_type&>(*this));
      return boost::apply_visitor (visitor_has_enabled,  transition().data());
    }

    const input_t & input() const
    {
      return input_;
    }

    input_t & input()
    {
      return input_;
    }

    const output_t & output() const
    {
      return output_;
    }

    output_t & output()
    {
      return output_;
    }

    template <typename Input>
    void
    input ( const Input & i )
    {
      unique_lock_t lock(*this);
      input_ = i;
    }

    template <typename Output>
    void
    output ( const Output & o )
    {
      unique_lock_t lock(*this);
      output_ = o;
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
    template <typename NetActivity>
    this_type
    create_activity_from_net_activity (NetActivity const & net_act, const id_type id)
    {
      this_type act(id);
      this->link_child(act);
      return act;
    }

    template <typename Activity>
    void link_child(Activity & act)
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
    void serialize (Archive & ar, const unsigned int)
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


  template <typename Transition, typename Token, typename Id, typename Traits>
  std::ostream & operator << ( std::ostream & os
                             , const activity_t<Transition, Token, Id, Traits> & act
                             )
  {
    typedef activity_t<Transition, Token, Id, Traits> activity_t;

    os << "{";
      os << "act, "
         << act.id()
         << ", "
         << "flags"
         << ", "
         << act.transition()
         << ", "
         ;

    os << "{input, "
       << "[";
    for (typename activity_t::input_t::const_iterator i (act.input().begin()); i != act.input().end(); ++i)
    {
      if (i != act.input().begin())
        os << ", ";
      os << we::type::detail::translate_port_to_name (act.transition(), i->second)
         << "=(" << i->first << ", " << i->second << ")";
    }
    os << "]";

    os << ", ";

    os << "{output, "
       << "[";
    for (typename activity_t::output_t::const_iterator o (act.output().begin()); o != act.output().end(); ++o)
    {
      if (o != act.output().begin())
        os << ", ";
      os << we::type::detail::translate_port_to_name (act.transition(), o->second) << "=(" << o->first << ", " << o->second << ")";
    }
    os << "]";

    os << "}";
    return os;
  }
}}}

#endif

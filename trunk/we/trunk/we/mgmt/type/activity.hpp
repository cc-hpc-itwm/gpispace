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
#include <boost/serialization/access.hpp>
#include <boost/random.hpp>

#include <we/type/id.hpp>
#include <we/mgmt/type/flags.hpp>
#include <we/mgmt/bits/traits.hpp>
#include <we/mgmt/bits/transition_visitors.hpp>

namespace we { namespace mgmt { namespace type {
  template <typename Transition>
  struct activity_traits
  {
    typedef petri_net::pid_t pid_t;
    typedef petri_net::pid_t activity_id_t;
    typedef Transition transition_type;
    typedef typename Transition::token_type token_type;

    typedef std::pair<token_type, pid_t> token_on_place_t;
    typedef std::vector<token_on_place_t> token_on_place_list_t;
    typedef token_on_place_list_t input_t;
    typedef token_on_place_list_t output_t;

    inline static
    activity_id_t invalid_id (void)
    {
      return ::we::mgmt::traits::def::id_traits<activity_id_t>::nil();
    }
  };

  template <typename Transition, typename Traits = activity_traits<Transition> >
  class activity_t
  {
    typedef activity_t<Transition, Traits> this_type;

  public:
    typedef Transition transition_type;
    typedef Traits traits_type;

    typedef typename traits_type::token_type token_type;
    typedef typename traits_type::token_on_place_t token_on_place_t;
    typedef typename traits_type::token_on_place_list_t token_on_place_list_t;
    typedef typename traits_type::input_t input_t;
    typedef typename traits_type::output_t output_t;
    typedef typename traits_type::pid_t pid_t;
    typedef typename traits_type::activity_id_t id_t;

    typedef boost::shared_lock<this_type> shared_lock_t;
    typedef boost::unique_lock<this_type> unique_lock_t;

    activity_t ()
      : id_ (traits_type::invalid_id())
    { }

    template <typename T>
    activity_t (const T & transition)
      : id_ (traits_type::invalid_id())
      , transition_ (transition)
    { }

    activity_t (const this_type & other)
      : id_ (other.id_)
      , flags_ (other.flags_)
      , transition_ (other.transition_)
      , input_ (other.input_)
      , output_ (other.output_)
    { }

    this_type & operator= (const this_type & other)
    {
      if (this != &other)
      {
        id_ = other.id_;
        flags_ = (other.flags_);
        transition_ = (other.transition_);
        input_ = (other.input_);
        output_ = (other.output_);
      }
      return *this;
    }

    inline
    void set_id (const id_t & new_id)
    {
      id_ = new_id;
    }

    inline
    id_t const & id (void) const
    {
      return id_;
    }

    inline
    const flags::flags_t & flags (void) const
    {
      return flags_;
    }

    inline
    bool is_alive() const
    {
      shared_lock_t lock(const_cast<this_type&>(*this));
      return ( flags::flag_traits<flags::flags_t>::is_alive (flags_) );
    }

    inline
    bool is_suspended() const
    {
      shared_lock_t lock(const_cast<this_type&>(*this));
      return ( flags::flag_traits<flags::flags_t>::is_suspended (flags_) );
    }

    inline
    bool is_cancelling() const
    {
      shared_lock_t lock(const_cast<this_type&>(*this));
      return ( flags::flag_traits<flags::flags_t>::is_cancelling (flags_) );
    }

    inline
    bool is_cancelled() const
    {
      shared_lock_t lock(const_cast<this_type&>(*this));
      return ( flags::flag_traits<flags::flags_t>::is_cancelled (flags_) );
    }

    inline
    bool is_failed() const
    {
      shared_lock_t lock(const_cast<this_type&>(*this));
      return ( flags::flag_traits<flags::flags_t>::is_failed (flags_) );
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

    inline
    std::string type_to_string (void) const
    {
      static const we::mgmt::visitor::type_to_string_visitor v;
      return boost::apply_visitor (v, transition_.data());
    }

    this_type
    extract()
    {
      unique_lock_t lock(*this);
      we::mgmt::visitor::activity_extractor<this_type, boost::mt19937>
        extract_activity(engine_);
      this_type act = boost::apply_visitor ( extract_activity
                                           , transition_.data()
                                           );
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
    }

    void
    inject_input ()
    {
      unique_lock_t lock(*this);
      we::mgmt::visitor::injector<this_type>
        inject_input (*this, input_);
      boost::apply_visitor ( inject_input
                           , transition_.data()
                           );
    }

    void
    collect_output ()
    {
      unique_lock_t lock(*this);
      we::mgmt::visitor::output_collector<this_type> collect_output (*this);
      boost::apply_visitor ( collect_output
                           , transition_.data()
                           );
    }

    template <typename Context>
    void execute ( Context & ctxt )
    {
      /* context requirements

      internal
      ========

      net:
        inject tokens into net
        ctxt.handle_internally (act, net)
            -> extractor

      expr:
        evaluate expression
        ctxt.handle_internally (act, expr)
            -> injector

      mod:
        prepare input
           [(token-on-place)], { place <-> name }
        ctxt.handle_internally (act, mod_call_t)

      external
      ========

        ctxt.handle_externally (act, net)
        ctxt.handle_externally (act, expr)
        ctxt.handle_externally (act, mod)

      */

      we::mgmt::visitor::executor<this_type, Context> visitor_executor (*this, ctxt);
      boost::apply_visitor (visitor_executor, transition().data());
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
    friend class boost::serialization::access;
    template<class Archive>
    void serialize (Archive & ar, const unsigned int)
    {
      ar & BOOST_SERIALIZATION_NVP(id_);
      ar & BOOST_SERIALIZATION_NVP(flags_);
      ar & BOOST_SERIALIZATION_NVP(transition_);
      ar & BOOST_SERIALIZATION_NVP(input_);
      ar & BOOST_SERIALIZATION_NVP(output_);
    }

  private:
    id_t id_;
    flags::flags_t flags_;
    mutable boost::shared_mutex mutex_;

    transition_type transition_;

    input_t input_;
    output_t output_;

    boost::mt19937 engine_;
  };

      template <typename Trans, typename Traits>
      inline bool operator==(const activity_t<Trans,Traits> & a, const activity_t<Trans,Traits> & b)
      {
        return a.id() == b.id();
      }
      template <typename Trans, typename Traits>
      inline std::size_t hash_value(activity_t<Trans, Traits> const & a)
      {
        boost::hash<typename activity_t<Trans, Traits>::id_t> hasher;
        return hasher(a.id());
      }

      namespace detail
      {
        template <typename Activity, typename Stream = std::ostream>
        struct printer
        {
          typedef printer<Activity, Stream> this_type;
          typedef typename Activity::token_on_place_list_t top_list_t;

          printer (const Activity & act, Stream & stream)
            : act_(act)
            , os (stream)
          { }

          this_type & operator << ( const top_list_t & top_list )
          {
            os << "[";

            for ( typename top_list_t::const_iterator top (top_list.begin())
                ; top != top_list.end()
                ; ++top
                )
            {
              if (top != top_list.begin())
                os << ", ";
              os << we::type::detail::translate_port_to_name (act_.transition(), top->second)
                 << "=(" << top->first << ", " << top->second << ")";
            }

            os << "]";
            return *this;
          }

          template <typename T>
          this_type & operator << ( const T & t )
          {
            os << t;
            return *this;
          }

          // make std::endl work
          this_type & operator << ( std::ostream & (*fn)(std::ostream &) )
          {
            fn (os);
            return *this;
          }

        private:
          const Activity & act_;
          Stream & os;
        };
      }


  template <typename Transition, typename Traits>
  std::ostream & operator << ( std::ostream & os
                             , const activity_t<Transition, Traits> & act
                             )
  {
    typedef activity_t<Transition, Traits> activity_t;

    os << "{";
      os << "act, "
         << act.flags()
         << ", "
         << act.transition()
         << ", "
         ;

    detail::printer<activity_t> printer (act, os);
    os << "{input, ";
    printer << act.input();
    os << "}";
    os << ", ";
    os << "{output, ";
    printer << act.output();
    os << "}";

    os << "}";

    return os;
  }
}}}

#endif

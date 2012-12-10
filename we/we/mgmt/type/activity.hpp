// {petry,rahn}@itwm.fhg.de

#ifndef WE_MGMT_TYPE_ACTIVITY_HPP
#define WE_MGMT_TYPE_ACTIVITY_HPP 1

#include <vector>
#include <boost/unordered_set.hpp>
#include <boost/bimap.hpp>
#include <boost/bimap/unordered_set_of.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/serialization/access.hpp>
#include <boost/random.hpp>

#include <we/type/id.hpp>
#include <we/type/transition.hpp>

#include <we/mgmt/type/flags.hpp>
#include <we/mgmt/bits/traits.hpp>
#include <we/mgmt/bits/transition_visitors.hpp>

#include <we/type/bits/transition/optimize.hpp>

namespace we { namespace mgmt { namespace type {
      namespace detail
      {
        template <typename Activity, typename Stream = std::ostream>
        struct printer
        {
          typedef printer<Activity, Stream> this_type;
          typedef typename Activity::token_on_port_list_t top_list_t;

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
              os << act_.transition().name_of_port (top->second)
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

  class activity_t
  {
  public:
    typedef std::pair<token::type, petri_net::port_id_type> token_on_port_t;
    typedef std::vector<token_on_port_t> token_on_port_list_t;
    typedef token_on_port_list_t input_t;
    typedef token_on_port_list_t output_t;

    typedef boost::unique_lock<boost::recursive_mutex> shared_lock_t;
    typedef boost::unique_lock<boost::recursive_mutex> unique_lock_t;

    activity_t ()
      : id_ (petri_net::activity_id_invalid())
    { }

    activity_t (const we::type::transition_t & transition)
      : id_ (petri_net::activity_id_invalid())
      , transition_ (transition)
    { }

    activity_t (const activity_t & other)
      : id_ (other.id_)
      , flags_ (other.flags_)
      , transition_ (other.transition_)
      , pending_input_ (other.pending_input_)
      , input_(other.input_)
      , output_ (other.output_)
    { }

    activity_t & operator= (const activity_t & other)
    {
      if (this != &other)
      {
        id_ = other.id_;
        flags_ = (other.flags_);
        transition_ = (other.transition_);
        pending_input_ = (other.pending_input_);
        input_ = (other.input_);
        output_ = (other.output_);
      }
      return *this;
    }

    inline
    void set_id (const petri_net::activity_id_type & new_id)
    {
      id_ = new_id;
    }

    inline
    petri_net::activity_id_type const & id (void) const
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
      shared_lock_t lock(mutex_);
      return ( flags::flag_traits<flags::flags_t>::is_alive (flags_) );
    }

    inline
    bool is_suspended() const
    {
      shared_lock_t lock(mutex_);
      return ( flags::flag_traits<flags::flags_t>::is_suspended (flags_) );
    }

    inline
    void set_suspended(bool value = true)
    {
      unique_lock_t lock(mutex_);
      flags::flag_traits<flags::flags_t>::set_suspended(flags_, value);
    }

    inline
    bool is_cancelling() const
    {
      shared_lock_t lock(mutex_);
      return ( flags::flag_traits<flags::flags_t>::is_cancelling (flags_) );
    }

    inline
    void set_cancelling(bool value = true)
    {
      unique_lock_t lock(mutex_);
      flags::flag_traits<flags::flags_t>::set_cancelling (flags_, value);
    }

    inline
    bool is_cancelled() const
    {
      shared_lock_t lock(mutex_);
      return ( flags::flag_traits<flags::flags_t>::is_cancelled (flags_) );
    }

    inline
    void set_cancelled(bool value = true)
    {
      unique_lock_t lock(mutex_);
      flags::flag_traits<flags::flags_t>::set_cancelled (flags_, value);
    }

    inline
    bool is_failed() const
    {
      shared_lock_t lock(mutex_);
      return ( flags::flag_traits<flags::flags_t>::is_failed (flags_) );
    }

    inline
    void set_failed(bool value = true)
    {
      unique_lock_t lock(mutex_);
      flags::flag_traits<flags::flags_t>::set_failed (flags_, value);
    }

    inline
    bool is_finished() const
    {
      shared_lock_t lock(mutex_);
      return ( flags::flag_traits<flags::flags_t>::is_finished (flags_) );
    }

    inline
    void set_finished(bool value = true)
    {
      unique_lock_t lock(mutex_);
      flags::flag_traits<flags::flags_t>::set_finished (flags_, value);
    }

    inline
    const we::type::transition_t & transition() const
    {
      shared_lock_t lock(mutex_);
      return transition_;
    }

    inline
    we::type::transition_t & transition()
    {
      unique_lock_t lock(mutex_);
      return transition_;
    }

    inline
    std::string type_to_string (void) const
    {
      static const we::mgmt::visitor::type_to_string_visitor v;
      return boost::apply_visitor (v, transition_.data());
    }

    activity_t
    extract()
    {
      unique_lock_t lock(mutex_);
      we::mgmt::visitor::activity_extractor<activity_t, boost::mt19937>
        extract_activity(engine_);
      activity_t act = boost::apply_visitor ( extract_activity
                                           , transition_.data()
                                           );
      return act;
    }

    void
    inject (activity_t const & subact)
    {
      unique_lock_t lock(mutex_);
      we::mgmt::visitor::activity_injector<activity_t>
        inject_activity (*this, subact);
      boost::apply_visitor ( inject_activity
                           , transition_.data()
                           , subact.transition_.data()
                           );
    }

    void
    inject_input ()
    {
      unique_lock_t lock(mutex_);
      we::mgmt::visitor::injector<activity_t>
        inject_input (*this, pending_input_);
      boost::apply_visitor ( inject_input
                           , transition_.data()
                           );
      std::copy ( pending_input_.begin()
                , pending_input_.end()
                , std::inserter (input_, input_.end())
                );
      pending_input_.clear();
    }

    void
    collect_output ()
    {
      unique_lock_t lock(mutex_);
      we::mgmt::visitor::output_collector<activity_t> collect_output (*this);
      boost::apply_visitor ( collect_output
                           , transition_.data()
                           );
    }

    template <typename Context>

    typename Context::result_type execute ( Context & ctxt )
    {
      unique_lock_t lock(mutex_);
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

      we::mgmt::visitor::executor<activity_t, Context> visitor_executor (*this, ctxt);
      return boost::apply_visitor (visitor_executor, transition().data());
    }

    bool
    can_fire (void) const
    {
      static const we::mgmt::visitor::can_fire visitor_can_fire;

      shared_lock_t lock(mutex_);
      return boost::apply_visitor (visitor_can_fire, transition().data());
    }

    const input_t & pending_input() const
    {
      shared_lock_t lock (mutex_);
      return pending_input_;
    }

    const input_t & input() const
    {
      shared_lock_t lock(mutex_);
      return input_;
    }

    void add_input (const input_t::value_type & inp)
    {
      unique_lock_t lock(mutex_);
      pending_input_.push_back (inp);
    }

    const output_t & output() const
    {
      shared_lock_t lock(mutex_);
      return output_;
    }

    void set_output (const output_t & outp)
    {
      unique_lock_t lock(mutex_);
      output_ = outp;
    }

    void add_output (const output_t::value_type & outp)
    {
      unique_lock_t lock(mutex_);
      output_.push_back (outp);
    }

    void writeTo (std::ostream & os) const
    {
      unique_lock_t lock(mutex_);

      os << "{";
      os << "act, "
         << flags()
         << ", "
         << transition()
         << ", "
        ;

      detail::printer<activity_t> printer (*this, os);
      os << "{input, ";
      printer << input();
      os << "}";
      os << "{pending, ";
      printer << pending_input();
      os << "}";
      os << ", ";
      os << "{output, ";
      printer << output();
      os << "}";

      os << "}";
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
  private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize (Archive & ar, const unsigned int)
    {
      unique_lock_t lock(mutex_);

      ar & BOOST_SERIALIZATION_NVP(id_);
      ar & BOOST_SERIALIZATION_NVP(flags_);
      ar & BOOST_SERIALIZATION_NVP(transition_);
      ar & BOOST_SERIALIZATION_NVP(pending_input_);
      ar & BOOST_SERIALIZATION_NVP(input_);
      ar & BOOST_SERIALIZATION_NVP(output_);
    }

  private:
    petri_net::activity_id_type id_;
    flags::flags_t flags_;
    mutable boost::recursive_mutex mutex_;

    we::type::transition_t transition_;

    input_t pending_input_;
    input_t input_;
    output_t output_;

    boost::mt19937 engine_;
  };

      inline bool operator==(const activity_t& a, const activity_t& b)
      {
        return a.id() == b.id();
      }


  inline std::ostream & operator << ( std::ostream & os
                                    , const activity_t & act
                                    )
  {
    act.writeTo (os);
    return os;
  }
}}}

#endif

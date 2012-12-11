// {petry,rahn}@itwm.fhg.de

#ifndef WE_MGMT_TYPE_ACTIVITY_HPP
#define WE_MGMT_TYPE_ACTIVITY_HPP 1

#include <we/type/id.hpp>
#include <we/type/transition.hpp>

#include <we/mgmt/type/flags.hpp>
#include <we/mgmt/bits/traits.hpp>
#include <we/mgmt/bits/transition_visitors.hpp>

#include <we/type/bits/transition/optimize.hpp>

#include <boost/unordered_set.hpp>
#include <boost/bimap.hpp>
#include <boost/bimap/unordered_set_of.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/serialization/access.hpp>
#include <boost/random.hpp>

#include <vector>

#include <iosfwd>

namespace we
{
  namespace mgmt
  {
    namespace type
    {

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
      : _id (petri_net::activity_id_invalid())
    { }

    activity_t (const we::type::transition_t & transition)
      : _id (petri_net::activity_id_invalid())
      , _transition (transition)
    { }

    activity_t (const activity_t & other)
      : _id (other._id)
      , _flags (other._flags)
      , _transition (other._transition)
      , _pending_input (other._pending_input)
      , _input(other._input)
      , _output (other._output)
    { }

    activity_t & operator= (const activity_t & other)
    {
      if (this != &other)
      {
        _id = other._id;
        _flags = (other._flags);
        _transition = (other._transition);
        _pending_input = (other._pending_input);
        _input = (other._input);
        _output = (other._output);
      }
      return *this;
    }

    void set_id (const petri_net::activity_id_type & new_id)
    {
      _id = new_id;
    }

    petri_net::activity_id_type const & id (void) const
    {
      return _id;
    }

    const flags::flags_t & flags (void) const
    {
      return _flags;
    }

    bool is_alive() const
    {
      shared_lock_t lock(_mutex);
      return (flags::is_alive (_flags));
    }

#define FLAG(_name)                             \
    bool is_ ## _name() const                   \
    {                                           \
      shared_lock_t lock(_mutex);               \
      return (flags::is_ ## _name (_flags));    \
    }                                           \
    void set_ ## _name (bool value = true)      \
    {                                           \
      unique_lock_t lock(_mutex);               \
      flags::set_ ## _name (_flags, value);     \
    }

    FLAG (suspended);
    FLAG (cancelling);
    FLAG (cancelled);
    FLAG (failed);
    FLAG (finished);
#undef FLAG

    //! \todo DIRTY! Why lock and return a ref? Eliminate!!
    const we::type::transition_t & transition() const
    {
      shared_lock_t lock(_mutex);
      return _transition;
    }

    //! \todo DIRTY! Why lock and return a ref? Eliminate!!
    we::type::transition_t & transition()
    {
      unique_lock_t lock(_mutex);
      return _transition;
    }

    std::string type_to_string (void) const
    {
      static const we::mgmt::visitor::type_to_string_visitor v;
      return boost::apply_visitor (v, _transition.data());
    }

    activity_t
    extract()
    {
      unique_lock_t lock(_mutex);
      we::mgmt::visitor::activity_extractor<activity_t, boost::mt19937>
        extract_activity(_engine);
      activity_t act = boost::apply_visitor ( extract_activity
                                           , _transition.data()
                                           );
      return act;
    }

    void
    inject (activity_t const & subact)
    {
      unique_lock_t lock(_mutex);
      we::mgmt::visitor::activity_injector<activity_t>
        inject_activity (*this, subact);
      boost::apply_visitor ( inject_activity
                           , _transition.data()
                           , subact._transition.data()
                           );
    }

    void
    inject_input ()
    {
      unique_lock_t lock(_mutex);
      we::mgmt::visitor::injector<activity_t>
        inject_input (*this, _pending_input);
      boost::apply_visitor ( inject_input
                           , _transition.data()
                           );
      std::copy ( _pending_input.begin()
                , _pending_input.end()
                , std::inserter (_input, _input.end())
                );
      _pending_input.clear();
    }

    void
    collect_output ()
    {
      unique_lock_t lock(_mutex);
      we::mgmt::visitor::output_collector<activity_t> collect_output (*this);
      boost::apply_visitor ( collect_output
                           , _transition.data()
                           );
    }

    template <typename Context>

    typename Context::result_type execute ( Context & ctxt )
    {
      unique_lock_t lock(_mutex);
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

      shared_lock_t lock(_mutex);
      return boost::apply_visitor (visitor_can_fire, transition().data());
    }

    const input_t & pending_input() const
    {
      shared_lock_t lock (_mutex);
      return _pending_input;
    }

    const input_t & input() const
    {
      shared_lock_t lock(_mutex);
      return _input;
    }

    void add_input (const input_t::value_type & inp)
    {
      unique_lock_t lock(_mutex);
      _pending_input.push_back (inp);
    }

    const output_t & output() const
    {
      shared_lock_t lock(_mutex);
      return _output;
    }

    void set_output (const output_t & outp)
    {
      unique_lock_t lock(_mutex);
      _output = outp;
    }

    void add_output (const output_t::value_type & outp)
    {
      unique_lock_t lock(_mutex);
      _output.push_back (outp);
    }

    void writeTo (std::ostream&) const;

    // **********************************
    //
    // Lockable concept implementation
    //
    // **********************************
    void lock()
    {
      _mutex.lock();
    }

    bool try_lock()
    {
      return _mutex.try_lock();
    }

    void unlock()
    {
      _mutex.unlock();
    }
  private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize (Archive & ar, const unsigned int)
    {
      unique_lock_t lock(_mutex);

      ar & BOOST_SERIALIZATION_NVP(_id);
      ar & BOOST_SERIALIZATION_NVP(_flags);
      ar & BOOST_SERIALIZATION_NVP(_transition);
      ar & BOOST_SERIALIZATION_NVP(_pending_input);
      ar & BOOST_SERIALIZATION_NVP(_input);
      ar & BOOST_SERIALIZATION_NVP(_output);
    }

  private:
    petri_net::activity_id_type _id;
    flags::flags_t _flags;
    mutable boost::recursive_mutex _mutex;

    we::type::transition_t _transition;

    input_t _pending_input;
    input_t _input;
    output_t _output;

    boost::mt19937 _engine;
  };

      namespace detail
      {
        struct printer
        {
          typedef activity_t::token_on_port_list_t top_list_t;

          printer (const activity_t&, std::ostream&);
          printer& operator<< (const top_list_t&);
          printer& operator<< (std::ostream & (*fn)(std::ostream&));

          template<typename T>
          printer& operator<< (const T& t)
          {
            _os << t;
            return *this;
          }

        private:
          const activity_t& _act;
          std::ostream& _os;
        };
      }

      bool operator== (const activity_t&, const activity_t&);
      std::ostream& operator<< (std::ostream &, const activity_t&);
    }
  }
}

#endif

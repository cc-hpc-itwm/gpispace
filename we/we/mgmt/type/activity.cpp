// {petry,rahn}@itwm.fhg.de

#include <we/net.hpp>

#include <we/mgmt/type/activity.hpp>

#include <boost/foreach.hpp>

#include <iostream>

namespace we
{
  namespace mgmt
  {
    namespace type
    {
      namespace
      {
        class visitor_activity_extractor
          : public boost::static_visitor<activity_t>
        {
        private:
          boost::mt19937& _engine;

        public:
          visitor_activity_extractor (boost::mt19937& engine)
            : _engine (engine)
          {}

          activity_t operator() (petri_net::net& net) const
          {
            const petri_net::net::activity_t net_act
              (net.extract_activity_random (_engine));

            activity_t act (net.get_transition (net_act.tid)) ;

            BOOST_FOREACH ( const petri_net::net::token_input_t& inp
                          , net_act.input
                          )
              {
                act.add_input
                  ( std::make_pair
                    ( inp.first
                    , act.transition().outer_to_inner (inp.second.first)
                    )
                  );
              }

            return act;
          }

          template<typename T>
          activity_t operator() (T&) const
          {
            throw std::runtime_error ("STRANGE: activity_extractor");
          }
        };
      }

      activity_t activity_t::extract()
      {
        unique_lock_t lock (_mutex);

        return boost::apply_visitor ( visitor_activity_extractor (_engine)
                                    , _transition.data()
                                    );
      }

      namespace
      {
        class visitor_activity_injector : public boost::static_visitor<>
        {
        private:
          activity_t& _parent;
          const activity_t& _child;

        public:
          visitor_activity_injector ( activity_t& parent
                                    , const activity_t& child
                                    )
            : _parent (parent)
            , _child (child)
          {}

          template<typename Child>
          void operator() (petri_net::net& parent, const Child&) const
          {
            visitor::inject_output_to_net ( parent
                                          , _child.transition()
                                          , _child.output()
                                          );
          }

          template <typename A, typename B>
          void operator() (const A&, const B&) const
          {
            throw std::runtime_error ("STRANGE: activity_injector");
          }
        };
      }

      void activity_t::inject (const activity_t& subact)
      {
        unique_lock_t lock (_mutex);

        boost::apply_visitor ( visitor_activity_injector (*this, subact)
                             , _transition.data()
                             , subact._transition.data()
                             );
      }

      void activity_t::inject_input()
      {
        unique_lock_t lock (_mutex);

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

      void activity_t::collect_output ()
      {
        unique_lock_t lock (_mutex);
        we::mgmt::visitor::output_collector<activity_t> collect_output (*this);
        boost::apply_visitor ( collect_output
                             , _transition.data()
                             );
      }

      void activity_t::set_id (const petri_net::activity_id_type& new_id)
      {
        _id = new_id;
      }

      const petri_net::activity_id_type& activity_t::id() const
      {
        return _id;
      }

      const flags::flags_t& activity_t::flags() const
      {
        return _flags;
      }

      bool activity_t::is_alive() const
      {
        shared_lock_t lock (_mutex);
        return (flags::is_alive (_flags));
      }

#define FLAG(_name)                                             \
      bool activity_t::is_ ## _name() const                     \
      {                                                         \
        shared_lock_t lock (_mutex);                            \
        return (flags::is_ ## _name (_flags));                  \
      }                                                         \
      void activity_t::set_ ## _name (bool value)               \
      {                                                         \
        unique_lock_t lock (_mutex);                            \
        flags::set_ ## _name (_flags, value);                   \
      }

      FLAG (suspended);
      FLAG (cancelling);
      FLAG (cancelled);
      FLAG (failed);
      FLAG (finished);
#undef FLAG

      const we::type::transition_t& activity_t::transition() const
      {
        shared_lock_t lock (_mutex);
        return _transition;
      }

      we::type::transition_t& activity_t::transition()
      {
        unique_lock_t lock (_mutex);
        return _transition;
      }

      namespace
      {
        class visitor_can_fire : public boost::static_visitor<bool>
        {
        public:
          bool operator () (const petri_net::net& net) const
          {
            return net.can_fire();
          }
          bool operator () (const we::type::module_call_t&) const
          {
            return false;
          }
          bool operator () (const we::type::expression_t&) const
          {
            return false;
          }
        };
      }

      bool activity_t::can_fire() const
      {
        shared_lock_t lock (_mutex);
        return boost::apply_visitor (visitor_can_fire(), transition().data());
      }

      const activity_t::input_t& activity_t::pending_input() const
      {
        shared_lock_t lock (_mutex);
        return _pending_input;
      }

      const activity_t::input_t& activity_t::input() const
      {
        shared_lock_t lock (_mutex);
        return _input;
      }

      void activity_t::add_input (const input_t::value_type& inp)
      {
        unique_lock_t lock (_mutex);
        _pending_input.push_back (inp);
      }

      const activity_t::output_t& activity_t::output() const
      {
        shared_lock_t lock (_mutex);
        return _output;
      }

      void activity_t::set_output (const output_t& outp)
      {
        unique_lock_t lock (_mutex);
        _output = outp;
      }

      void activity_t::add_output (const output_t::value_type& outp)
      {
        unique_lock_t lock (_mutex);
        _output.push_back (outp);
      }

      void activity_t::lock()
      {
        _mutex.lock();
      }
      void activity_t::unlock()
      {
        _mutex.unlock();
      }

      namespace
      {
        class visitor_type_to_string : public boost::static_visitor<std::string>
        {
        public:
          std::string operator () (const petri_net::net&) const
          {
            return "net";
          }
          std::string operator () (const we::type::module_call_t&) const
          {
            return "module";
          }
          std::string operator () (const we::type::expression_t&) const
          {
            return "expr";
          }
        };
      }

      std::string activity_t::type_to_string() const
      {
        return boost::apply_visitor ( visitor_type_to_string()
                                    , _transition.data()
                                    );
      }

      namespace detail
      {
        printer::printer (const activity_t& act, std::ostream& os)
          : _act (act)
          , _os (os)
        {}

        printer& printer::operator<< (const top_list_t& top_list)
        {
          bool first (true);

          _os << "[";

          BOOST_FOREACH (const activity_t::token_on_port_t& top, top_list)
            {
              if (first)
                {
                  first = false;
                }
              else
                {
                  _os << ", ";
                }

              _os << _act.transition().name_of_port (top.second)
                  << "=(" << top.first << ", " << top.second << ")";
            }

          _os << "]";

          return *this;
        }

        printer& printer::operator<< (std::ostream& (*fn)(std::ostream&))
        {
          fn (_os);
          return *this;
        }
      }

      void activity_t::writeTo (std::ostream& os) const
      {
        unique_lock_t lock (_mutex);

        os << "{";
        os << "act, "
           << flags()
           << ", "
           << transition()
           << ", "
          ;

        detail::printer printer (*this, os);
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

      bool operator== (const activity_t& a, const activity_t& b)
      {
        return a.id() == b.id();
      }

      std::ostream& operator<< (std::ostream& os, const activity_t& act)
      {
        act.writeTo (os);
        return os;
      }
    }
  }
}

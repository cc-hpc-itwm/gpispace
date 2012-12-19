// {petry,rahn}@itwm.fhg.de

#include <we/type/net.hpp>

#include <we/type/transition.hpp>

#include <we/mgmt/context.hpp>

#include <we/mgmt/type/activity.hpp>

#include <fhg/util/show.hpp>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/foreach.hpp>

#include <sstream>
#include <iostream>

namespace we
{
  namespace mgmt
  {
    namespace type
    {
      activity_t::activity_t ()
        : _id (petri_net::activity_id_invalid())
      {}

      activity_t::activity_t (const we::type::transition_t& transition)
        : _id (petri_net::activity_id_invalid())
        , _transition (transition)
      {}

      activity_t::activity_t (const activity_t& other)
        : _id (other._id)
        , _flags (other._flags)
        , _transition (other._transition)
        , _pending_input (other._pending_input)
        , _input(other._input)
        , _output (other._output)
      {}

      activity_t& activity_t::operator= (const activity_t& other)
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

      std::string activity_t::to_string() const
      {
        std::ostringstream oss;
        boost::archive::text_oarchive ar (oss);
        ar << BOOST_SERIALIZATION_NVP (*this);
        return oss.str();
      }

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
                    , act.transition().outer_to_inner (inp.second)
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

          void operator() (petri_net::net& parent) const
          {
            BOOST_FOREACH ( const activity_t::token_on_port_t& top
                          , _child.output()
                          )
              {
                try
                  {
                    token::put ( parent
                               , _child.transition().inner_to_outer (top.second)
                               , top.first
                               );
                  }
                catch (const we::type::exception::not_connected<petri_net::port_id_type>&)
                  {
                    std::cerr << "W: transition generated output, but port is not connected:"
                              << " trans=\"" << _child.transition().name() << "\""
                              << " port="
                              << _child.transition().name_of_port (top.second)
                              << "(" << top.second << ")"
                              << " token=" << fhg::util::show (top.first)
                              << std::endl;
                  }
              }
          }

          template <typename A>
          void operator() (A&) const
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
                             );
      }

      namespace
      {
        class visitor_injector : public boost::static_visitor<void>
        {
        private:
          we::type::transition_t& _transition;
          const activity_t::input_t& _input;

        public:
          visitor_injector ( activity_t& activity
                           , const activity_t::input_t& input
                           )
            : _transition (activity.transition())
            , _input (input)
          {}

          void operator() (petri_net::net& net) const
          {
            BOOST_FOREACH (const activity_t::token_on_port_t& inp, _input)
              {
                const petri_net::port_id_type port_id (inp.second);

                if (_transition.get_port (port_id).has_associated_place())
                  {
                    token::put
                      ( net
                      , _transition.get_port (port_id).associated_place()
                      , inp.first
                      );
                  }
              }
          }

          template<typename T>
          void operator() (T&) const
          {}
        };
      }

      void activity_t::inject_input()
      {
        unique_lock_t lock (_mutex);

        boost::apply_visitor ( visitor_injector (*this, _pending_input)
                             , _transition.data()
                             );

        std::copy ( _pending_input.begin()
                  , _pending_input.end()
                  , std::inserter (_input, _input.end())
                  );

        _pending_input.clear();
      }

      namespace
      {
        class visitor_collect_output : public boost::static_visitor<>
        {
        private:
          activity_t& _activity;

        public:
          visitor_collect_output (activity_t& activity)
            : _activity (activity)
          {}

          void operator() (petri_net::net& net) const
          {
            typedef we::type::transition_t::const_iterator port_iterator;

            for ( port_iterator port_it (_activity.transition().ports_begin())
                ; port_it != _activity.transition().ports_end()
                ; ++port_it
                )
              {
                if (port_it->second.is_output())
                  {
                    if (port_it->second.has_associated_place())
                      {
                        const petri_net::port_id_type port_id (port_it->first);
                        const petri_net::place_id_type pid (port_it->second.associated_place());

                        for ( petri_net::net::token_place_it top
                                (net.get_token (pid))
                            ; top.has_more ()
                            ; ++top
                            )
                          {
                            _activity.add_output (activity_t::output_t::value_type (*top, port_id));
                          }

                        net.delete_all_token (pid);
                      }
                    else
                      {
                        throw std::runtime_error
                          ( "output port ("
                          + fhg::util::show (port_it->first)
                          + ", " + fhg::util::show (port_it->second) + ") "
                          + "is not associated with any place!"
                          );
                      }
                  }
              }
          }

          template<typename T>
          void operator() (T&) const
          {
            return;
          }
        };

      }

      void activity_t::collect_output ()
      {
        unique_lock_t lock (_mutex);

        boost::apply_visitor ( visitor_collect_output (*this)
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
        class executor : public boost::static_visitor<int>
        {
        private:
          type::activity_t& _activity;
          context* _ctxt;

        public:
          executor (type::activity_t& activity, context* ctxt)
            : _activity (activity)
            , _ctxt (ctxt)
          {}

          int operator () (petri_net::net& net) const
          {
            if (_activity.transition().is_internal())
              {
                return _ctxt->handle_internally (_activity, net);
              }
            else
              {
                return _ctxt->handle_externally (_activity, net);
              }
          }

          int operator() (we::type::module_call_t & mod) const
          {
            return _ctxt->handle_externally (_activity, mod);
          }

          int operator() (we::type::expression_t & expr) const
          {
            expr::eval::context context;

            for ( type::activity_t::input_t::const_iterator top
                    (_activity.input().begin())
                ; top != _activity.input().end()
                ; ++top
                )
              {
                context.bind ( _activity.transition().name_of_port (top->second)
                             , top->first.value
                             );
              }

            expr.ast ().eval_all (context);

            for ( we::type::transition_t::const_iterator port_it
                    (_activity.transition().ports_begin())
                ; port_it != _activity.transition().ports_end()
                ; ++port_it
                )
              {
                if (port_it->second.is_output())
                  {
                    _activity.add_output
                      ( type::activity_t::output_t::value_type
                      ( token::type ( port_it->second.name()
                                    , port_it->second.signature()
                                    , context
                                    )
                      , port_it->first
                      )
                      );
                  }
              }

            return _ctxt->handle_internally (_activity, expr);
          }
        };
      }

      int activity_t::execute (context* ctxt)
      {
        unique_lock_t lock (_mutex);
        return boost::apply_visitor
          (executor (*this, ctxt), transition().data());
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

      std::ostream& activity_t::print ( std::ostream& os
                                      , const token_on_port_list_t& top_list
                                      ) const
      {
        bool first (true);

        os << "[";

        BOOST_FOREACH (const token_on_port_t& top, top_list)
          {
            if (first)
              {
                first = false;
              }
            else
              {
                os << ", ";
              }

            os << transition().name_of_port (top.second)
               << "=(" << top.first << ", " << top.second << ")";
          }

        os << "]";

        return os;
      }

      void activity_t::writeTo (std::ostream& os) const
      {
        unique_lock_t lock (_mutex);

        os << "{";

        os << "act, " << flags() << ", " << transition() << ", ";

        os << "{input, "; print (os, input()); os << "}";
        os << ", {pending, "; print (os, pending_input()); os << "}";
        os << ", {output, "; print (os, output()); os << "}";

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

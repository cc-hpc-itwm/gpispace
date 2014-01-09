// {petry,rahn}@itwm.fhg.de

#include <we/type/net.hpp>

#include <we/type/transition.hpp>

#include <we/expr/eval/context.hpp>

#include <we/mgmt/context.hpp>

#include <we/type/activity.hpp>

#include <we/type/value/show.hpp>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

#include <sstream>
#include <iostream>
#include <fstream>

namespace we
{
    namespace type
    {
      activity_t::activity_t ()
      {}

      activity_t::activity_t
        ( const we::type::transition_t& transition
        , boost::optional<petri_net::transition_id_type> const& transition_id
        )
        : _transition (transition)
        , _transition_id (transition_id)
      {}

      namespace
      {
        void decode (std::istream& s, activity_t& t)
        {
          try
            {
              boost::archive::text_iarchive ar (s);

              ar >> BOOST_SERIALIZATION_NVP (t);
            }
          catch (std::exception const &ex)
            {
              throw std::runtime_error
                (std::string ("deserialization error: ") + ex.what ());
            }
        }
      }

      activity_t::activity_t (const boost::filesystem::path& path)
      {
        std::ifstream stream (path.string().c_str());

        if (!stream)
          {
            throw std::runtime_error
              ("failed to open " + path.string() + "for reading");
          }

        decode (stream, *this);
      }

      activity_t::activity_t (std::istream& stream)
      {
        decode (stream, *this);
      }

      activity_t::activity_t (const std::string& s)
      {
        std::istringstream stream (s);

        decode (stream, *this);
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
            return net.extract_activity_random (_engine);
          }

          template<typename T>
          activity_t operator() (T&) const
          {
            throw std::runtime_error ("STRANGE: activity_extractor");
          }
        };
      }

      activity_t activity_t::extract (boost::mt19937& engine)
      {
        return boost::apply_visitor ( visitor_activity_extractor (engine)
                                    , _transition.data()
                                    );
      }

      namespace
      {
        class visitor_activity_injector : public boost::static_visitor<>
        {
        private:
          const activity_t& _child;

        public:
          visitor_activity_injector (const activity_t& child)
            : _child (child)
          {}

          void operator() (petri_net::net& parent) const
          {
            BOOST_FOREACH ( const activity_t::token_on_port_t& top
                          , _child.output()
                          )
              {
                parent.put_value
                  ( *parent.port_to_place (*_child.transition_id(), top.second)
                  , top.first
                  );
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
        boost::apply_visitor ( visitor_activity_injector (subact)
                             , _transition.data()
                             );
      }

      namespace
      {
        class visitor_add_input : public boost::static_visitor<>
        {
        private:
          we::type::transition_t const& _transition;
          petri_net::port_id_type const& _port_id;
          pnet::type::value::value_type const& _value;

        public:
          visitor_add_input
            ( we::type::transition_t const& transition
            , petri_net::port_id_type const& port_id
            , pnet::type::value::value_type const& value
            )
            : _transition (transition)
            , _port_id (port_id)
            , _value (value)
          {}

          void operator() (petri_net::net& net) const
          {
            if (_transition.ports().at (_port_id).has_associated_place())
            {
              net.put_value
                ( _transition.ports().at (_port_id).associated_place()
                , _value
                );
            }
          }

          template<typename T>
          void operator() (T&) const
          {}
        };
      }

      void activity_t::add_input
        ( petri_net::port_id_type const& port_id
        , pnet::type::value::value_type const& value
        )
      {
        boost::apply_visitor ( visitor_add_input (_transition, port_id, value)
                             , _transition.data()
                             );

        _input.push_back (input_t::value_type (value, port_id));
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
            BOOST_FOREACH
              ( we::type::transition_t::port_map_t::value_type const& p
              , _activity.transition().ports()
              )
            {
              if (p.second.is_output())
              {
                if (p.second.has_associated_place())
                {
                  const petri_net::port_id_type& port_id (p.first);
                  const petri_net::place_id_type& pid
                    (p.second.associated_place());

                  BOOST_FOREACH ( const pnet::type::value::value_type& token
                                , net.get_token (pid)
                                )
                  {
                    _activity.add_output (port_id, token);
                  }

                  net.delete_all_token (pid);
                }
                else
                {
                  throw std::runtime_error
                    ( "output port ("
                    + boost::lexical_cast<std::string> (p.first)
                    + ", " + boost::lexical_cast<std::string> (p.second) + ") "
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
        boost::apply_visitor ( visitor_collect_output (*this)
                             , _transition.data()
                             );
      }

      const we::type::transition_t& activity_t::transition() const
      {
        return _transition;
      }

      we::type::transition_t& activity_t::transition()
      {
        return _transition;
      }

      namespace
      {
        class executor : public boost::static_visitor<void>
        {
        private:
          type::activity_t& _activity;
          context* _ctxt;

        public:
          executor (type::activity_t& activity, context* ctxt)
            : _activity (activity)
            , _ctxt (ctxt)
          {}

          void operator () (petri_net::net const& net) const
          {
            if (_activity.transition().is_internal())
              {
                _ctxt->handle_internally (_activity, net);
              }
            else
              {
                _ctxt->handle_externally (_activity, net);
              }
          }

          void operator() (we::type::module_call_t const& mod) const
          {
            _ctxt->handle_externally (_activity, mod);
          }

          void operator() (we::type::expression_t const& expr) const
          {
            expr::eval::context context;

            for ( type::activity_t::input_t::const_iterator top
                    (_activity.input().begin())
                ; top != _activity.input().end()
                ; ++top
                )
              {
                context.bind_ref
                  ( _activity.transition().ports().at (top->second).name()
                  , top->first
                  );
              }

            expr.ast ().eval_all (context);

            BOOST_FOREACH
              ( we::type::transition_t::port_map_t::value_type const& p
              , _activity.transition().ports()
              )
            {
              if (p.second.is_output())
              {
                _activity.add_output (p.first, context.value (p.second.name()));
              }
            }

            return _ctxt->handle_internally (_activity, expr);
          }
        };
      }

      void activity_t::execute (context* ctxt)
      {
        boost::apply_visitor
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
        return boost::apply_visitor (visitor_can_fire(), transition().data());
      }

      const activity_t::input_t& activity_t::input() const
      {
        return _input;
      }

      const activity_t::output_t& activity_t::output() const
      {
        return _output;
      }

      void activity_t::add_output
        ( petri_net::port_id_type const& port_id
        , pnet::type::value::value_type const& value
        )
      {
        _output.push_back (output_t::value_type (value, port_id));
      }

      boost::optional<petri_net::transition_id_type> const&
        activity_t::transition_id() const
      {
        return _transition_id;
      }
    }
}

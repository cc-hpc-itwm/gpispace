// {petry,rahn}@itwm.fhg.de

#include <we/type/net.hpp>

#include <we/type/transition.hpp>

#include <we/expr/eval/context.hpp>

#include <we/context.hpp>

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
        , boost::optional<we::transition_id_type> const& transition_id
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
        class visitor_activity_injector : public boost::static_visitor<>
        {
        private:
          const activity_t& _child;

        public:
          visitor_activity_injector (const activity_t& child)
            : _child (child)
          {}

          void operator() (we::net& parent) const
          {
            BOOST_FOREACH ( const activity_t::token_on_port_t& top
                          , _child.output()
                          )
              {
                parent.put_value
                  ( parent.port_to_place().at (*_child.transition_id())
                  .left.find (top.second)->get_right()
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
          we::port_id_type const& _port_id;
          pnet::type::value::value_type const& _value;

        public:
          visitor_add_input
            ( we::type::transition_t const& transition
            , we::port_id_type const& port_id
            , pnet::type::value::value_type const& value
            )
            : _transition (transition)
            , _port_id (port_id)
            , _value (value)
          {}

          void operator() (we::net& net) const
          {
            if (_transition.ports().at (_port_id).associated_place())
            {
              net.put_value
                ( *_transition.ports().at (_port_id).associated_place()
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
        ( we::port_id_type const& port_id
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

          void operator() (we::net& net) const
          {
            BOOST_FOREACH
              ( we::type::transition_t::port_map_t::value_type const& p
              , _activity.transition().ports()
              )
            {
              if (p.second.is_output())
              {
                if (p.second.associated_place())
                {
                  const we::port_id_type& port_id (p.first);
                  const we::place_id_type& pid
                    (*p.second.associated_place());

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
                    + ", " + p.second.name() + ") "
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

          void operator () (we::net const& net) const
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

      const activity_t::input_t& activity_t::input() const
      {
        return _input;
      }

      const activity_t::output_t& activity_t::output() const
      {
        return _output;
      }

      void activity_t::add_output
        ( we::port_id_type const& port_id
        , pnet::type::value::value_type const& value
        )
      {
        _output.push_back (output_t::value_type (value, port_id));
      }

      boost::optional<we::transition_id_type> const&
        activity_t::transition_id() const
      {
        return _transition_id;
      }
    }
}

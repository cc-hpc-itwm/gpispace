// {petry,rahn}@itwm.fhg.de

#include <we/type/net.hpp>

#include <we/type/transition.hpp>

#include <we/expr/eval/context.hpp>

#include <we/context.hpp>

#include <we/type/activity.hpp>

#include <we/type/value/show.hpp>

#include <util-generic/nest_exceptions.hpp>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include <algorithm>
#include <cassert>
#include <sstream>
#include <iostream>
#include <fstream>

namespace we
{
    namespace type
    {
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
              ( ( boost::format ("deserialization error: '%1%'") % ex.what()
                ).str()
              );
          }
        }
      }

      activity_t::activity_t (const boost::filesystem::path& path)
      {
        std::ifstream stream (path.string().c_str());

        if (!stream)
        {
          throw std::runtime_error
            ((boost::format ("could not open '%1%' for reading") % path).str());
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

      void activity_t::inject
        ( const activity_t& child
        , we::workflow_response_callback workflow_response
        )
      {
        we::type::net_type& net
          (boost::get<we::type::net_type> (_transition.data()));

        for (const activity_t::token_on_port_t& top : child.output())
        {
          if ( net.port_to_place().count (*child.transition_id())
             && net.port_to_place().at (*child.transition_id())
                .left.count (top.second)
             )
          {
            net.put_value
              ( net.port_to_place().at (*child.transition_id())
              .left.find (top.second)->get_right()
              , top.first
              );
          }
          else
          {
            fhg::util::nest_exceptions<std::runtime_error>
              ( [&]
                {
                  assert ( net.port_to_response().at (*child.transition_id())
                         . left.count (top.second)
                         );

                  pnet::type::value::value_type const description
                    ([this, &net, &child, &top]
                     {
                       std::string const to
                         ( net.port_to_response().at (*child.transition_id())
                         . left.find (top.second)->get_right()
                         );

                       we::port_id_type const input_port_id
                         (child.transition().input_port_by_name (to));

                       return std::find_if
                         ( child._input.begin(), child._input.end()
                         , [&input_port_id] (token_on_port_t const& input_top)
                           {
                             return input_top.second == input_port_id;
                           }
                         )->first;
                     }()
                    );

                  workflow_response (description, top.first);
                }
              , "inject result: sending workflow response failed"
              );
          }
        }
      }

      void activity_t::add_input
        ( we::port_id_type const& port_id
        , pnet::type::value::value_type const& value
        )
      {
        if (_transition.net())
        {
          //! \todo is the conditional neccessary? isn't is ensured already?
          if (_transition.ports_input().at (port_id).associated_place())
          {
            boost::get<we::type::net_type>(_transition.data()).put_value
              ( *_transition.ports_input().at (port_id).associated_place()
              , value
              );
          }
        }
        else
        {
          _input.push_back (input_t::value_type (value, port_id));
        }
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

          void operator () (we::type::net_type const& net) const
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
                  ( _activity.transition().ports_input().at (top->second).name()
                  , top->first
                  );
              }

            expr.ast ().eval_all (context);

            for ( we::type::transition_t::port_map_t::value_type const& p
                : _activity.transition().ports_output()
                )
            {
                _activity.add_output (p.first, context.value (p.second.name()));
            }
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

      activity_t::output_t activity_t::output() const
      {
        if (_transition.net())
        {
          output_t output;

          for ( we::type::transition_t::port_map_t::value_type const& p
              : _transition.ports_output()
              )
          {
                const we::port_id_type& port_id (p.first);
                const we::place_id_type& pid
                  (*p.second.associated_place());

                for ( const pnet::type::value::value_type& token
                    : _transition.net()->get_token (pid)
                    )
                {
                  output.push_back (std::make_pair (token, port_id));
                }
          }

          return output;
        }
        else
        {
          return _output;
        }
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

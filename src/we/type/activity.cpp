// {petry,rahn}@itwm.fhg.de

#include <we/type/net.hpp>

#include <we/type/transition.hpp>

#include <we/type/activity.hpp>

#include <util-generic/nest_exceptions.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/range/adaptor/map.hpp>

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
          _input.emplace_back (value, port_id);
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
                    | boost::adaptors::map_values
                    )
                {
                  output.emplace_back (token, port_id);
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
        _output.emplace_back (value, port_id);
      }
      void activity_t::add_output (expr::eval::context const& output)
      {
        for ( std::pair<port_id_type, port_t> const& port_by_id
            : _transition.ports_output()
            )
        {
          add_output
            (port_by_id.first, output.value (port_by_id.second.name()));
        }
      }

      bool activity_t::output_missing() const
      {
        output_t const out (output());

        if (out.size() < _transition.ports_output().size())
        {
          return true;
        }

        std::unordered_set<port_id_type> port_ids_with_output;

        for (port_id_type port_id : out | boost::adaptors::map_values)
        {
          port_ids_with_output.emplace (port_id);
        }

        return port_ids_with_output.size() != _transition.ports_output().size();
      }

      boost::optional<we::transition_id_type> const&
        activity_t::transition_id() const
      {
        return _transition_id;
      }

      expr::eval::context activity_t::evaluation_context() const
      {
        expr::eval::context context;

        for ( std::pair< pnet::type::value::value_type
                       , we::port_id_type
                       > const& token_on_port
            : _input
            )
        {
          context.bind_ref
            ( transition().ports_input().at (token_on_port.second).name()
            , token_on_port.first
            );
        }

        return context;
      }

      namespace
      {
        template<typename T>
          boost::optional<T> eval_schedule_data
            ( transition_t const& transition
            , expr::eval::context context
            , const std::string& key
            )
        {
          boost::optional<const property::value_type&> expression_value
            (transition.prop().get ({"fhg", "drts", "schedule", key}));

          if (!expression_value)
          {
            return boost::none;
          }

          expression_t const expression
            (boost::get<std::string> (expression_value.get()));

          return boost::get<T> (expression.ast().eval_all (context));
        }
      }

      schedule_data activity_t::get_schedule_data() const
      {
        return { eval_schedule_data<unsigned long>
                   (_transition, evaluation_context(), "num_worker")
               };
      }
    }
}

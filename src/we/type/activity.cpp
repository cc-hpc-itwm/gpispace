#include <we/type/activity.hpp>

#include <we/loader/module_call.hpp>
#include <we/type/net.hpp>
#include <we/type/transition.hpp>

#include <fhg/assert.hpp>
#include <fhg/util/starts_with.hpp>

#include <gpi-space/pc/client/api.hpp>

#include <drts/private/scoped_allocation.hpp>

#include <drts/worker/context.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/format.hpp>
#include <boost/range/adaptor/map.hpp>

#include <algorithm>
#include <cassert>
#include <exception>
#include <fstream>
#include <iostream>
#include <list>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>

namespace
{
  struct wfe_exec_context : public boost::static_visitor<expr::eval::context>
  {
    wfe_exec_context
      ( we::loader::loader& loader
      , gpi::pc::client::api_t /*const*/* virtual_memory
      , gspc::scoped_allocation /*const*/* shared_memory
      , boost::optional<std::string> target_implementation
      , drts::worker::context* worker_context
      , expr::eval::context const& evaluation_context
      , std::string const& name
      )
      : _loader (loader)
      , _virtual_memory (virtual_memory)
      , _shared_memory (shared_memory)
      , _target_implementation (target_implementation)
      , _worker_context (worker_context)
      , _evaluation_context (evaluation_context)
      , _name (name)
    {}

    expr::eval::context operator() (we::type::net_type const&) const
    {
      throw std::logic_error ("wfe_exec_context (net)");
    }
    expr::eval::context operator() (we::type::expression_t const&) const
    {
      throw std::logic_error ("wfe_exec_context (expression)");
    }

    expr::eval::context operator() (we::type::module_call_t const& mod) const
    {
      try
      {
        return we::loader::module_call
          ( _loader
          , _virtual_memory
          , _shared_memory
          , _worker_context
          , _evaluation_context
          , mod
          );
      }
      catch (drts::worker::context::cancelled const&)
      {
        throw;
      }
      catch (...)
      {
        std::throw_with_nested
          ( std::runtime_error
              ("call to '" + mod.module() + "::" + mod.function() + "' failed")
          );
      }
    }

    expr::eval::context operator()
      (we::type::multi_module_call_t const& multi_mod) const
    {
      if (!_target_implementation)
      {
        std::throw_with_nested
          ( std::runtime_error
              ( "no target selected for multi-module transition '"
                + _name
                + "' failed"
              )
          );
      }

      auto const& mod_it = multi_mod.find (*_target_implementation);
      if (mod_it == multi_mod.end())
      {
        std::throw_with_nested
          ( std::runtime_error
              ( "no module for target '" + *_target_implementation + "' found"
                " found in multi-module transition '"
                + _name
              )
          );
      }

      return (*this) (mod_it->second);
    }


  private:
    we::loader::loader& _loader;
    gpi::pc::client::api_t /*const*/* _virtual_memory;
    gspc::scoped_allocation /*const*/* _shared_memory;
    boost::optional<std::string> _target_implementation;
    drts::worker::context* _worker_context;
    expr::eval::context const& _evaluation_context;
    std::string const& _name;
  };
}

namespace we
{
  namespace type
  {
    activity_t::activity_t (we::type::transition_t transition)
      : activity_t (std::move (transition), boost::none)
    {}
    activity_t::activity_t ( TESTING_ONLY
                           , we::type::transition_t transition
                           , we::transition_id_type transition_id
                           )
      : activity_t ( std::move (transition)
                   , boost::optional<we::transition_id_type> (transition_id)
                   )
    {}
    activity_t::activity_t
      ( we::type::transition_t transition
      , boost::optional<we::transition_id_type> transition_id
      )
        : _transition (std::move (transition))
        , _transition_id (std::move (transition_id))
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
      ( std::string const& port_name
      , pnet::type::value::value_type const& value
      )
    {
      return add_input (_transition.input_port_by_name (port_name), value);
    }
    void activity_t::add_input
      ( we::port_id_type const& port_id
      , pnet::type::value::value_type const& value
      )
    {
      if (_evaluation_context_requested)
      {
        throw std::logic_error
          ("add_input after evaluation context has been requested");
      }

      if (_transition.net())
      {
        //! \todo is the conditional neccessary? isn't is ensured already?
        if (_transition.ports_input().at (port_id).associated_place())
        {
          _transition.mutable_net().put_value
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

    std::string const& activity_t::name() const
    {
      return _transition.name();
    }
    bool activity_t::handle_by_workflow_engine() const
    {
      return !!_transition.net();
    }

    boost::optional<eureka_id_type> const& activity_t::eureka_id() const
    {
      return _transition.eureka_id();
    }

    void activity_t::set_wait_for_output()
    {
      return _transition.set_property ({"drts", "wait_for_output"}, true);;
    }
    void activity_t::put_token
      (std::string place_name, pnet::type::value::value_type const& token)
    {
      return _transition.mutable_net()
        . put_token (std::move (place_name), token)
        ;
    }
    void activity_t::inject ( activity_t const& result
                            , workflow_response_callback workflow_response
                            , eureka_response_callback eureka_response
                            )
    {
      return _transition.mutable_net()
        . inject ( *result._transition_id
                 , result.output()
                 , result._input
                 , std::move (workflow_response)
                 , std::move (eureka_response)
                 );
    }
    boost::optional<activity_t>
      activity_t::extract
      ( std::mt19937& random_engine
      , workflow_response_callback const& workflow_response
      , eureka_response_callback const& eureka_response
      , gspc::we::plugin::Plugins& plugins
      , gspc::we::plugin::PutToken put_token
      )
    {
      return _transition.mutable_net()
        . fire_expressions_and_extract_activity_random
          ( random_engine
          , workflow_response
          , eureka_response
          , plugins
          , std::move (put_token)
          )
        ;
    }

    const TokensOnPorts& activity_t::input() const
    {
      return _input;
    }

    std::multimap<std::string, pnet::type::value::value_type>
      activity_t::result() const
    {
      std::multimap<std::string, pnet::type::value::value_type> result;

      for (auto const& value_on_port : output())
      {
        result.emplace
          ( _transition.ports_output().at (value_on_port.second).name()
          , value_on_port.first
          );
      }

      return result;
    }

    TokensOnPorts activity_t::output() const
    {
      if (_transition.net())
      {
        TokensOnPorts output;

        for ( we::type::transition_t::port_map_t::value_type const& p
            : _transition.ports_output()
            )
        {
          const we::port_id_type& port_id (p.first);
          const we::place_id_type& pid
            (*p.second.associated_place());

          for ( const pnet::type::value::value_type& token
              : _transition.net()->get_token (pid) | boost::adaptors::map_values
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

    void activity_t::add_output_TESTING_ONLY
      ( std::string const& port_name
      , pnet::type::value::value_type const& value
      )
    {
      add_output (_transition.output_port_by_name (port_name), value);
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
      if (_transition.net())
      {
        throw std::logic_error ("add_output for subnetwork");
      }

      for (auto const& port_by_id : _transition.ports_output())
      {
        add_output
          (port_by_id.first, output.value ({port_by_id.second.name()}));
      }
    }

    bool activity_t::wait_for_output() const
    {
      if (!_transition.prop().is_true ({"drts", "wait_for_output"}))
      {
        return false;
      }

      TokensOnPorts const out (output());

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

    void activity_t::execute
      ( we::loader::loader& loader
      , gpi::pc::client::api_t /*const*/ * virtual_memory
      , gspc::scoped_allocation /* const */ * shared_memory
      , boost::optional<std::string> target_implementation
      , drts::worker::context* worker_context
      )
    {
      add_output
        ( boost::apply_visitor
          ( wfe_exec_context ( loader
                             , virtual_memory
                             , shared_memory
                             , std::move (target_implementation)
                             , worker_context
                             , evaluation_context()
                             , name()
                             )
          , _transition.data()
          )
        );
    }

    expr::eval::context activity_t::evaluation_context() const
    {
      if (_transition.net())
      {
        throw std::logic_error ("evaluation context for net is undefined");
      }

      _evaluation_context_requested = true;

      expr::eval::context context;

      for (auto const& token_on_port : _input)
      {
        context.bind_ref
          ( _transition.ports_input().at (token_on_port.second).name()
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

      boost::optional<std::string> eval_dynamic_requirement
        ( transition_t const& transition
        , expr::eval::context context
        )
      {
        boost::optional<const property::value_type&> expression_value
          (transition.prop().get ({"fhg", "drts", "require", "dynamic_requirement"}));

        if (!expression_value)
        {
          return boost::none;
        }

        expression_t const expression
          (boost::get<std::string> (expression_value.get()));

        return boost::get<std::string> (expression.ast().eval_all (context));
      }
    }

    Requirements_and_preferences activity_t::requirements_and_preferences
      (gpi::pc::client::api_t* virtual_memory_api) const
    {
      auto const context (evaluation_context());

      schedule_data const schedule_data
        ( eval_schedule_data<unsigned long>
            (_transition, context, "num_worker")
        );

      auto const num_required_workers (schedule_data.num_worker());

      if ( num_required_workers
         && *num_required_workers > 1
         && !_transition.preferences().empty()
         )
      {
        throw std::runtime_error
          ("Not allowed to use coallocation for activities with multiple module implementations!");
      }

      const double computational_cost (1.0); //!Note: use here an adequate cost provided by we! (can be the wall time)

      auto requirements (_transition.requirements());

      auto dynamic_requirement
        (eval_dynamic_requirement (_transition, context));

      if (dynamic_requirement)
      {
        requirements.emplace_back (dynamic_requirement.get(), true);
      }

      return
        { requirements
        , std::move (schedule_data)
        , [&]
          {
            //! \todo Move to gpi::pc::client::api_t
            if (!_transition.module_call())
            {
              return null_transfer_cost;
            }

            auto vm_transfers (_transition.module_call()->gets (context));

            auto puts_before
              (_transition.module_call()->puts_evaluated_before_call (context));

            vm_transfers.splice (vm_transfers.end(), puts_before);

            if (vm_transfers.empty())
            {
              return null_transfer_cost;
            }

            if (!virtual_memory_api)
            {
              throw std::logic_error
                ("vmem transfers without vmem knowledge in agent");
            }

            return virtual_memory_api->transfer_costs (vm_transfers);
          }()
        , computational_cost
        , !_transition.module_call()
          ? 0UL
          : _transition.module_call()->memory_buffer_size_total (context)
        , _transition.preferences()
        };
    }

    std::list<we::type::preference_t>
      const activity_t::preferences_TESTING_ONLY() const
    {
      return _transition.preferences();
    }

    namespace
    {
      std::string wrapped_activity_prefix()
      {
        return "_wrap_";
      }

      std::string wrapped_name (port_t const& port)
      {
        return (port.is_output() ? "_out_" : "_in_") + port.name();
      }
    }

    activity_t activity_t::wrap() const
    {
      if (_transition.net())
      {
        return *this;
      }

      we::type::net_type net;

      auto const transition_id (net.add_transition (_transition));

      fhg_assert (_transition.ports_tunnel().size() == 0);

      std::unordered_map<std::string, we::place_id_type> place_ids;

      for (auto const& p : _transition.ports_input())
      {
        auto const place_id
          (net.add_place (place::type ( wrapped_name (p.second)
                                      , p.second.signature()
                                      , boost::none
                                      )
                         )
          );

        net.add_connection ( we::edge::PT
                           , transition_id
                           , place_id
                           , p.first
                           , we::type::property::type()
                           );

        place_ids.emplace (wrapped_name (p.second), place_id);
      }

      for (auto const& p : _transition.ports_output())
      {
        auto const place_id
          (net.add_place (place::type ( wrapped_name (p.second)
                                      , p.second.signature()
                                      , boost::none
                                      )
                         )
          );

        net.add_connection ( we::edge::TP
                           , transition_id
                           , place_id
                           , p.first
                           , we::type::property::type()
                           );

        place_ids.emplace (wrapped_name (p.second), place_id);
      }

      for (auto const& top : _input)
      {
        auto const& port (_transition.ports_input().at (top.second));

        net.put_value (place_ids.find (wrapped_name (port))->second, top.first);
      }

      //! \todo copy output too

      we::type::transition_t const
        transition_net_wrapper ( wrapped_activity_prefix() + name()
                               , net
                               , boost::none
                               , we::type::property::type()
                               , we::priority_type()
                               );

      return activity_t {transition_net_wrapper, _transition_id};
    }

    activity_t activity_t::unwrap() const
    {
      if (!fhg::util::starts_with (wrapped_activity_prefix(), name()))
      {
        return *this;
      }

      auto const& net (*_transition.net());
      auto const& transition_inner (net.transitions().begin()->second);
      auto const& transition_id_inner (net.transitions().begin()->first);

      type::activity_t activity_inner (transition_inner, _transition_id);

      for (auto const& p : transition_inner.ports_output())
      {
        auto const place_id
          (net.port_to_place().at (transition_id_inner).at (p.first).first);

        for ( auto const& token
            : net.get_token (place_id) | boost::adaptors::map_values
            )
        {
          activity_inner.add_output (p.first, token);
        }
      }

      //! \todo copy input too

      return activity_inner;
    }
  }
}

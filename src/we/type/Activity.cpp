// Copyright (C) 2012-2015,2019-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/we/type/Activity.hpp>

#include <gspc/we/loader/module_call.hpp>
#include <gspc/we/type/Transition.hpp>
#include <gspc/we/type/net.hpp>
#include <gspc/we/type/schedule_data.hpp>

#include <gspc/assert.hpp>
#include <gspc/util/starts_with.hpp>

#include <gspc/iml/stubs.hpp>

#if GSPC_WITH_IML
#include <gspc/iml/Client.hpp>
#endif
#include <gspc/iml/macros.hpp>
#if GSPC_WITH_IML
#include <gspc/iml/SharedMemoryAllocation.hpp>
#endif

#include <gspc/drts/worker/context.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include <gspc/util/fmt/std/filesystem/path.formatter.hpp>
#include <algorithm>
#include <cassert>
#include <exception>
#include <fmt/core.h>
#include <filesystem>
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
  struct wfe_exec_context : public ::boost::static_visitor<gspc::we::expr::eval::context>
  {
    wfe_exec_context
      ( gspc::we::loader::loader& loader
      , gspc::iml::Client /*const*/* virtual_memory
      , gspc::iml::SharedMemoryAllocation /*const*/* shared_memory
      , std::optional<std::string> target_implementation
      , drts::worker::context* worker_context
      , gspc::we::expr::eval::context const& evaluation_context
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

    gspc::we::expr::eval::context operator() (gspc::we::type::net_type const&) const
    {
      throw std::logic_error ("wfe_exec_context (net)");
    }
    gspc::we::expr::eval::context operator() (gspc::we::type::Expression const&) const
    {
      throw std::logic_error ("wfe_exec_context (expression)");
    }

    gspc::we::expr::eval::context operator() (gspc::we::type::ModuleCall const& mod) const
    {
      try
      {
        return gspc::we::loader::module_call
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

    gspc::we::expr::eval::context operator()
      (gspc::we::type::MultiModuleCall const& multi_mod) const
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
    gspc::we::loader::loader& _loader;
    gspc::iml::Client /*const*/* _virtual_memory;
    gspc::iml::SharedMemoryAllocation /*const*/* _shared_memory;
    std::optional<std::string> _target_implementation;
    drts::worker::context* _worker_context;
    gspc::we::expr::eval::context const& _evaluation_context;
    std::string const& _name;
  };
}


  namespace gspc::we::type
  {
    Activity::Activity (Transition transition)
      : Activity (std::move (transition), std::nullopt)
    {}
    Activity::Activity ( TESTING_ONLY
                           , Transition transition
                           , we::transition_id_type transition_id
                           )
      : Activity ( std::move (transition)
                   , std::optional<we::transition_id_type> (transition_id)
                   )
    {}
    Activity::Activity
      ( Transition transition
      , std::optional<we::transition_id_type> transition_id
      )
        : _transition (std::move (transition))
        , _transition_id (std::move (transition_id))
    {}

    namespace
    {
      void decode (std::istream& s, Activity& t)
      {
        try
        {
          ::boost::archive::text_iarchive ar (s);

          ar >> t;
        }
        catch (std::exception const& ex)
        {
          throw std::runtime_error
            {fmt::format ("deserialization error: '{}'", ex.what())};
        }
      }
    }

    Activity::Activity (std::filesystem::path const& path)
    {
      std::ifstream stream (path);

      if (!stream)
      {
        throw std::runtime_error
          {fmt::format ("could not open '{}' for reading", path.string())};
      }

      decode (stream, *this);
    }

    Activity::Activity (std::istream& stream)
    {
      decode (stream, *this);
    }

    Activity::Activity (std::string const& s)
    {
      std::istringstream stream (s);

      decode (stream, *this);
    }

    std::string Activity::to_string() const
    {
      std::ostringstream oss;
      ::boost::archive::text_oarchive ar (oss);
      ar << *this;
      return oss.str();
    }

    void Activity::add_input
      ( std::string const& port_name
      , pnet::type::value::value_type const& value
      )
    {
      return add_input (transition().input_port_by_name (port_name), value);
    }
    void Activity::add_input
      ( we::port_id_type const& port_id
      , pnet::type::value::value_type const& value
      )
    {
      if (_evaluation_context_requested)
      {
        throw std::logic_error
          ("add_input after evaluation context has been requested");
      }

      if (transition().net())
      {
        //! \todo is the conditional neccessary? isn't is ensured already?
        if (transition().ports_input().at (port_id).associated_place())
        {
          mutable_transition().mutable_net().put_value
            ( *transition().ports_input().at (port_id).associated_place()
            , value
            );
        }
      }
      else
      {
        _input.emplace_back (TokenOnPort {value, port_id});
      }
    }

    Transition const& Activity::transition() const
    {
      return _transition;
    }
    Transition& Activity::mutable_transition()
    {
      return _transition;
    }

    std::string const& Activity::name() const
    {
      return transition().name();
    }
    bool Activity::handle_by_workflow_engine() const
    {
      return !!transition().net();
    }

    std::optional<eureka_id_type> const& Activity::eureka_id()
    {
      if (!_eureka_id)
      {
        if (auto id = transition().eureka_id())
        {
          auto context (evaluation_context());

          _eureka_id = ::boost::get<eureka_id_type>
            (Expression (*id).ast().eval_all (context));
        }
      }

      return _eureka_id;
    }

    void Activity::set_wait_for_output()
    {
      return mutable_transition().set_property ({"drts", "wait_for_output"}, true);
    }
    void Activity::put_token
      (std::string place_name, pnet::type::value::value_type const& token)
    {
      return mutable_transition().mutable_net()
        . put_token (std::move (place_name), token)
        ;
    }
    void Activity::inject ( Activity const& result
                            , workflow_response_callback workflow_response
                            , eureka_response_callback eureka_response
                            )
    {
      return mutable_transition().mutable_net()
        . inject ( *result._transition_id
                 , result.output()
                 , result._input
                 , std::move (workflow_response)
                 , std::move (eureka_response)
                 );
    }
    std::optional<Activity>
      Activity::extract
      ( std::mt19937& random_engine
      , workflow_response_callback const& workflow_response
      , eureka_response_callback const& eureka_response
      , we::plugin::Plugins& plugins
      , we::plugin::PutToken put_token
      )
    {
      return mutable_transition().mutable_net()
        . fire_expressions_and_extract_activity_random
          ( random_engine
          , workflow_response
          , eureka_response
          , plugins
          , std::move (put_token)
          )
        ;
    }

    TokensOnPorts const& Activity::input() const
    {
      return _input;
    }

    std::multimap<std::string, pnet::type::value::value_type>
      Activity::result() const
    {
      std::multimap<std::string, pnet::type::value::value_type> result;

      for (auto const& output : output())
      {
        result.emplace
          ( transition().ports_output().at (output._port_id).name()
          , output._token
          );
      }

      return result;
    }

    TokensOnPorts Activity::output() const
    {
      if (transition().net())
      {
        TokensOnPorts output;

        for ( Transition::PortByID::value_type const& p
            : transition().ports_output()
            )
        {
          we::port_id_type const& port_id (p.first);
          we::place_id_type const& pid
            (*p.second.associated_place());

          for ( auto const& [_ignore, token]
              : transition().net()->get().get_token (pid)
              )
          {
            output.emplace_back (TokenOnPort {token, port_id});
          }
        }

        return output;
      }
      else
      {
        return _output;
      }
    }

    void Activity::add_output_TESTING_ONLY
      ( std::string const& port_name
      , pnet::type::value::value_type const& value
      )
    {
      add_output (transition().output_port_by_name (port_name), value);
    }
    void Activity::add_output
      ( we::port_id_type const& port_id
      , pnet::type::value::value_type const& value
      )
    {
      _output.emplace_back (TokenOnPort {value, port_id});
    }
    void Activity::add_output (expr::eval::context const& output)
    {
      if (transition().net())
      {
        throw std::logic_error ("add_output for subnetwork");
      }

      for (auto const& port_by_id : transition().ports_output())
      {
        add_output
          (port_by_id.first, output.value ({port_by_id.second.name()}));
      }
    }

    bool Activity::wait_for_output() const
    {
      if (!transition().prop().is_true ({"drts", "wait_for_output"}))
      {
        return false;
      }

      TokensOnPorts const out (output());

      if (out.size() < transition().ports_output().size())
      {
        return true;
      }

      std::unordered_set<port_id_type> port_ids_with_output;

      for (auto const& [_,port_id] : out)
      {
        port_ids_with_output.emplace (port_id);
      }

      return port_ids_with_output.size() != transition().ports_output().size();
    }

    bool Activity::might_use_virtual_memory() const
    {
      return transition().might_use_virtual_memory();
    }

    bool Activity::might_have_tasks_requiring_multiple_workers() const
    {
      return transition().might_have_tasks_requiring_multiple_workers();
    }

    bool Activity::might_use_modules_with_multiple_implementations() const
    {
      return transition().might_use_modules_with_multiple_implementations();
    }

    void Activity::execute
      ( we::loader::loader& loader
      , iml::Client /*const*/ * virtual_memory
      , iml::SharedMemoryAllocation /* const */ * shared_memory
      , std::optional<std::string> target_implementation
      , drts::worker::context* worker_context
      )
    {
      add_output
        ( ::boost::apply_visitor
          ( wfe_exec_context ( loader
                             , virtual_memory
                             , shared_memory
                             , std::move (target_implementation)
                             , worker_context
                             , evaluation_context()
                             , name()
                             )
          , transition().data()
          )
        );
    }

    expr::eval::context Activity::evaluation_context()
    {
      if (transition().net())
      {
        throw std::logic_error ("evaluation context for net is undefined");
      }

      _evaluation_context_requested = true;

      expr::eval::context context;

      for (auto const& input : _input)
      {
        context.bind_ref
          ( transition().ports_input().at (input._port_id).name()
          , input._token
          );
      }

      return context;
    }

    namespace
    {
      template<typename T>
        std::optional<T> evaluate_property
        ( Transition const& transition
        , expr::eval::context context
        , property::path_type const& path
        )
      {
        if (auto const expression = transition.prop().get (path))
        {
          return ::boost::get<T>
            ( Expression (::boost::get<std::string> (expression->get()))
            . ast()
            . eval_all (context)
            );
        }

        return {};
      }
    }

    #if GSPC_WITH_IML
    namespace
    {
      std::list<iml::MemoryRegion> define_memory_regions
        (std::list<std::pair<we::local::range, we::global::range>> const& transfers)
      {
        std::list<iml::MemoryRegion> regions;
        for (auto const& [_ignore, range] : transfers)
        {
          regions.emplace_back
            ( iml::MemoryLocation (range.handle().name(), range.offset())
            , range.size()
            );
        }

        return regions;
      }
    }
    #endif

    Requirements_and_preferences
      Activity::requirements_and_preferences
        ( iml::Client* IF_GSPC_WITH_IML (virtual_memory_api)
        )
    {
      auto const context (evaluation_context());

      auto const num_worker
        ( evaluate_property<unsigned long>
            ( transition()
            , context
            , {"fhg", "drts", "schedule", "num_worker"}
            )
        );

      if (  !!num_worker
         && *num_worker > 1
         && !transition().preferences().empty()
         )
      {
        throw std::runtime_error
          ("Not allowed to use coallocation for activities with multiple module implementations!");
      }

      auto const maximum_number_of_retries
        ( evaluate_property<unsigned long>
            ( transition()
            , context
            , {"fhg", "drts", "schedule", "maximum_number_of_retries"}
            )
        );

      const double computational_cost (1.0); //!Note: use here an adequate cost provided by we! (can be the wall time)

      auto requirements (transition().requirements());

      if ( auto const dynamic_requirement
         = evaluate_property<std::string>
             ( transition()
             , context
             , {"fhg", "drts", "require", "dynamic_requirement"}
             )
         )
      {
        requirements.emplace_back (dynamic_requirement.value());
      }

      return
        { requirements
        , {num_worker, maximum_number_of_retries}
        , [&]() -> decltype (null_transfer_cost)
          {
            if (!transition().module_call())
            {
              return null_transfer_cost;
            }

            auto vm_transfers (transition().module_call()->get().gets (context));

            auto puts_before
              (transition().module_call()->get().puts_evaluated_before_call (context));

            vm_transfers.splice (vm_transfers.end(), puts_before);

            if (vm_transfers.empty())
            {
              return null_transfer_cost;
            }

            #if GSPC_WITH_IML
            if (!virtual_memory_api)
            {
              throw std::logic_error
                ("vmem transfers without vmem knowledge in agent");
            }

            return [ costs
                   = virtual_memory_api->transfer_costs
                       (define_memory_regions (vm_transfers))
                   ] (std::string const& host)
                   {
                     return costs.at (host);
                   };
            #else
            return null_transfer_cost;
            #endif
          }()
        , computational_cost
        , !transition().module_call()
          ? 0UL
          : transition().module_call()->get().memory_buffer_size_total (context)
        , transition().preferences()
        };
    }

    std::list<Preference>
      Activity::preferences_TESTING_ONLY() const
    {
      return transition().preferences();
    }

    namespace
    {
      std::string wrapped_activity_prefix()
      {
        return "_wrap_";
      }

      std::string wrapped_name (Port const& port)
      {
        return (port.is_output() ? "_out_" : "_in_") + port.name();
      }
    }

    Activity Activity::wrap() &&
    {
      if (transition().net())
      {
        return std::move (*this);
      }

      net_type net;

      auto const transition_id (net.add_transition (transition()));

      gspc_assert (transition().ports_tunnel().size() == 0);

      std::unordered_map<std::string, we::place_id_type> place_ids;

      for (auto const& p : transition().ports_input())
      {
        auto const place_id
          (net.add_place (place::type ( wrapped_name (p.second)
                                      , p.second.signature()
                                      , std::nullopt
                                      , std::nullopt
                                      , property::type{}
                                      , place::type::Generator::No{}
                                      )
                         )
          );

        net.add_connection ( we::edge::PT{}
                           , transition_id
                           , place_id
                           , p.first
                           , property::type()
                           );

        place_ids.emplace (wrapped_name (p.second), place_id);
      }

      for (auto const& p : transition().ports_output())
      {
        auto const place_id
          (net.add_place (place::type ( wrapped_name (p.second)
                                      , p.second.signature()
                                      , std::nullopt
                                      , std::nullopt
                                      , property::type{}
                                      , place::type::Generator::No{}
                                      )
                         )
          );

        net.add_connection ( we::edge::TP{}
                           , transition_id
                           , place_id
                           , p.first
                           , property::type()
                           );

        place_ids.emplace (wrapped_name (p.second), place_id);
      }

      for (auto const& input : _input)
      {
        auto const& port (transition().ports_input().at (input._port_id));

        // Bypass shared reference tracking - wrapper nets are temporary
        // execution contexts and don't contain the actual cleanup places.
        // The parent net handles all reference counting.
        net.put_value_without_tracking
          ( place_ids.find (wrapped_name (port))->second
          , input._token
          );
      }

      //! \todo copy output too

      Transition const
        transition_net_wrapper ( wrapped_activity_prefix() + name()
                               , net
                               , std::nullopt
                               , property::type()
                               , we::priority_type()
                               , std::optional<eureka_id_type>{}
                               , std::list<Preference>{}
                               , track_shared{}
                               );

      return Activity {transition_net_wrapper, _transition_id};
    }

    Activity Activity::unwrap() &&
    {
      if (!util::starts_with (wrapped_activity_prefix(), name()))
      {
        return std::move (*this);
      }

      auto const& net (transition().net()->get());
      auto const& transition_inner (net.transitions().begin()->second);
      auto const& transition_id_inner (net.transitions().begin()->first);

      type::Activity activity_inner (transition_inner, _transition_id);

      for (auto const& p : transition_inner.ports_output())
      {
        auto const place_id
          (net.port_to_place().at (transition_id_inner).at (p.first)._place_id);

        for (auto const& [_ignore, token] : net.get_token (place_id))
        {
          activity_inner.add_output (p.first, token);
        }
      }

      //! \todo copy input too

      return activity_inner;
    }
  }

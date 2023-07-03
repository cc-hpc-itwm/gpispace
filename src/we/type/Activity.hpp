// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <we/type/Activity.fwd.hpp>

#include <we/eureka_response.hpp>
#include <we/expr/eval/context.hpp>
#include <we/loader/loader.hpp>
#include <we/plugin/Plugins.hpp>
#include <we/type/Transition.hpp>
#include <we/type/eureka.hpp>
#include <we/type/value/serialize.hpp>
#include <we/workflow_response.hpp>

#include <drts/worker/context_fwd.hpp>

#include <sdpa/requirements_and_preferences.hpp>

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include <boost/serialization/access.hpp>

#include <iosfwd>
#include <map>
#include <random>
#include <string>
#include <vector>

namespace iml
{
  class Client;
  class SharedMemoryAllocation;
}

//! is: Activity.fwd.hpp
namespace we
{
  namespace type
  {
    class Activity;

    using TokenOnPort = std::pair< pnet::type::value::value_type
                                 , we::port_id_type
                                 >;
    using TokensOnPorts = std::vector<TokenOnPort>;
  }
}

namespace we
{
  namespace type
  {
    struct TESTING_ONLY{};

    class Activity
    {
    public:
      explicit Activity () = default;
      explicit Activity (we::type::Transition);

      explicit Activity (::boost::filesystem::path const&);
      explicit Activity (std::istream&);
      explicit Activity (std::string const&);

      Activity (Activity const&) = default;
      Activity& operator= (Activity const&) = delete;

      Activity (Activity&&) = default;
      Activity& operator= (Activity&&) = default;

      ~Activity() = default;

      std::string to_string() const;

      we::type::Transition const& transition() const;

      std::string const& name() const;
      bool handle_by_workflow_engine() const;

      //! evaluates the eureka expression once and memorizes the value
      //! for subsequent calls
      ::boost::optional<eureka_id_type> const& eureka_id();

      void set_wait_for_output();
      void put_token
        (std::string place_name, pnet::type::value::value_type const&);
      void inject ( Activity const&
                  , workflow_response_callback
                  , eureka_response_callback
                  );
      ::boost::optional<Activity>
        extract ( std::mt19937&
                , workflow_response_callback const&
                , eureka_response_callback const&
                , gspc::we::plugin::Plugins&
                , gspc::we::plugin::PutToken
                );

      Activity wrap() &&;
      Activity unwrap() &&;

      TokensOnPorts const& input() const;
      void add_input
        ( std::string const& port_name
        , pnet::type::value::value_type const&
        );

      std::multimap<std::string, pnet::type::value::value_type>
        result() const;

      TokensOnPorts output() const;

      bool wait_for_output() const;
      bool might_use_virtual_memory() const;
      bool might_have_tasks_requiring_multiple_workers() const;
      bool might_use_modules_with_multiple_implementations() const;

      void execute
        ( we::loader::loader&
        , iml::Client /*const*/ *
        , iml::SharedMemoryAllocation /* const */ *
        , ::boost::optional<std::string> target_implementation
        , drts::worker::context*
        );

      Requirements_and_preferences requirements_and_preferences
        (iml::Client*);

      explicit Activity
        ( TESTING_ONLY
        , we::type::Transition
        , we::transition_id_type
        );

      void add_output_TESTING_ONLY
        ( std::string const& port_name
        , pnet::type::value::value_type const&
        );
      std::list<we::type::Preference> preferences_TESTING_ONLY() const;

    private:
      we::type::Transition& mutable_transition();

      we::type::Transition _transition;
      ::boost::optional<we::transition_id_type> _transition_id;

      friend class net_type;
      TokensOnPorts _input;
      TokensOnPorts _output;

      bool _evaluation_context_requested {false};
      ::boost::optional<eureka_id_type> _eureka_id;

      explicit Activity
        ( we::type::Transition
        , ::boost::optional<we::transition_id_type>
        );

      friend class ::boost::serialization::access;
      template<class Archive>
        void serialize (Archive&, unsigned int);

      void add_input
        ( we::port_id_type const&
        , pnet::type::value::value_type const&
        );
      void add_output
        ( we::port_id_type const&
        , pnet::type::value::value_type const&
        );
      void add_output (expr::eval::context const&);

      //! \note context contains references to input
      expr::eval::context evaluation_context();
    };
  }
}

#include <we/type/Activity.ipp>

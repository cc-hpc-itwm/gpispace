// Copyright (C) 2010,2012-2013,2015,2020-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/we/type/Activity.fwd.hpp>

#include <gspc/we/eureka_response.hpp>
#include <gspc/we/expr/eval/context.hpp>
#include <gspc/we/loader/loader.hpp>
#include <gspc/we/plugin/Plugins.hpp>
#include <gspc/we/type/Transition.hpp>
#include <gspc/we/type/eureka.hpp>
#include <gspc/we/type/value/serialize.hpp>
#include <gspc/we/workflow_response.hpp>

#include <gspc/drts/worker/context_fwd.hpp>

#include <gspc/we/type/requirements_and_preferences.hpp>

#include <filesystem>
#include <optional>
#include <boost/serialization/access.hpp>

#include <iosfwd>
#include <map>
#include <random>
#include <string>
#include <vector>

namespace gspc::iml
{
  class Client;
  class SharedMemoryAllocation;
}


  namespace gspc::we::type
  {
    struct TESTING_ONLY{};

    class Activity
    {
    public:
      explicit Activity () = default;
      explicit Activity (Transition);

      explicit Activity (std::filesystem::path const&);
      explicit Activity (std::istream&);
      explicit Activity (std::string const&);

      Activity (Activity const&) = default;
      Activity& operator= (Activity const&) = delete;

      Activity (Activity&&) = default;
      Activity& operator= (Activity&&) = default;

      ~Activity() = default;

      std::string to_string() const;

      Transition const& transition() const;

      std::string const& name() const;
      bool handle_by_workflow_engine() const;

      //! evaluates the eureka expression once and memorizes the value
      //! for subsequent calls
      std::optional<eureka_id_type> const& eureka_id();

      void set_wait_for_output();
      void put_token
        (std::string place_name, pnet::type::value::value_type const&);
      void inject ( Activity const&
                  , workflow_response_callback
                  , eureka_response_callback
                  );
      std::optional<Activity>
        extract ( std::mt19937&
                , workflow_response_callback const&
                , eureka_response_callback const&
                , we::plugin::Plugins&
                , we::plugin::PutToken
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
        , std::optional<std::string> target_implementation
        , drts::worker::context*
        );

      Requirements_and_preferences requirements_and_preferences
        (iml::Client*);

      explicit Activity
        ( TESTING_ONLY
        , Transition
        , we::transition_id_type
        );

      void add_output_TESTING_ONLY
        ( std::string const& port_name
        , pnet::type::value::value_type const&
        );
      std::list<Preference> preferences_TESTING_ONLY() const;

    private:
      Transition& mutable_transition();

      Transition _transition;
      std::optional<we::transition_id_type> _transition_id;

      friend class net_type;
      TokensOnPorts _input;
      TokensOnPorts _output;

      bool _evaluation_context_requested {false};
      std::optional<eureka_id_type> _eureka_id;

      explicit Activity
        ( Transition
        , std::optional<we::transition_id_type>
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


#include <gspc/we/type/Activity.ipp>

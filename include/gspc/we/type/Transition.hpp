// Copyright (C) 2010,2012-2015,2019-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/we/expr/eval/context.hpp>
#include <gspc/we/type/Expression.hpp>
#include <gspc/we/type/ModuleCall.hpp>
#include <gspc/we/type/Port.hpp>
#include <gspc/we/type/Requirement.hpp>
#include <gspc/we/type/eureka.hpp>
#include <gspc/we/type/id.hpp>
#include <gspc/we/type/net.fwd.hpp>
#include <gspc/we/type/property.hpp>
#include <gspc/we/type/value.hpp>

#include <functional>
#include <optional>

#include <gspc/util/serialization/std/optional.hpp>
#include <optional>
#include <boost/serialization/list.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/variant.hpp>
#include <boost/variant/recursive_wrapper.hpp>

#include <unordered_map>
#include <unordered_set>


  namespace gspc::we::type
  {
    using Preference = std::string;
    using MultiModuleCall = std::unordered_map <Preference, ModuleCall>;

    // Flags for shared value reference counting optimization.
    // When input/output is false, the corresponding reference
    // counting is skipped.
    struct track_shared
    {
      bool input {false};
      bool output {false};

      template<typename Archive>
        void serialize (Archive& ar, unsigned int)
      {
        ar & input;
        ar & output;
      }
    };

    struct Transition
    {
    private:
      using data_type = ::boost::variant
                      < ModuleCall
                      , MultiModuleCall
                      , Expression
                      , ::boost::recursive_wrapper<net_type>
                      >
        ;

    public:
      using PortByID = std::unordered_map<we::port_id_type, Port>;

      Transition();

      template <typename Type>
      Transition ( std::string const& name
                   , Type const& typ
                   , std::optional<Expression> const& _condition
                   , property::type const& prop
                   , we::priority_type priority
                   , std::optional<eureka_id_type> const& _eureka_id
                   , std::list<Preference> const& preferences
                   , struct track_shared track_shared
                   )
        try
          :  name_  (name)
          ,  data_  (typ)
          ,  condition_ (_condition)
          ,  _ports_input()
          ,  _ports_output()
          ,  _ports_tunnel()
          ,  port_id_counter_  (0)
          ,  prop_(prop)
          ,  _requirements()
          ,  _preferences  (preferences)
          ,  _priority  (priority)
          ,  eureka_id_ (_eureka_id)
          , _track_shared {track_shared}
      {
        if (preferences.size() && data_.type() != typeid (MultiModuleCall))
        {
          throw std::runtime_error
            ("preferences defined without multiple modules with target");
        }
      }
      catch (...)
      {
        std::throw_with_nested
          (std::runtime_error ("Failed to create transition '" + name + "'"));
      }

      std::string const& name() const;

      data_type const& data() const;
      net_type& mutable_net();

      std::optional<std::reference_wrapper<Expression const>> expression() const;
      std::optional<std::reference_wrapper<net_type const>> net() const;
      std::optional<std::reference_wrapper<ModuleCall const>> module_call() const;

      std::optional<Expression> const& condition() const;
      std::optional<eureka_id_type> const& eureka_id() const;

      we::port_id_type add_port (Port const&);

      we::port_id_type input_port_by_name (std::string const&) const;
      we::port_id_type const& output_port_by_name (std::string const&) const;

      PortByID const& ports_input() const;
      PortByID const& ports_output() const;
      PortByID const& ports_tunnel() const;

      property::type const& prop() const;

      std::list<Requirement> const& requirements() const;
      std::list<Preference> const& preferences() const;
      void add_requirement (Requirement const&);

      we::priority_type priority() const;

      // Shared value tracking flags for performance optimization.
      // When false, shared value reference counting is skipped.
      struct track_shared track_shared() const
      {
        return _track_shared;
      }

      void set_property ( property::path_type const& path
                        , property::value_type const& value
                        )
      {
        prop_.set (path, value);
      }

      Transition& assert_correct_expression_types();

      bool might_use_virtual_memory() const;
      bool might_have_tasks_requiring_multiple_workers() const;
      bool might_use_modules_with_multiple_implementations() const;

    private:
      std::string name_;
      data_type data_;
      std::optional<Expression> condition_;

      PortByID _ports_input;
      PortByID _ports_output;
      PortByID _ports_tunnel;
      we::port_id_type port_id_counter_;

      property::type prop_;

      std::list<Requirement> _requirements;
      std::list<Preference> _preferences;
      we::priority_type _priority;

      std::optional<eureka_id_type> eureka_id_;

      struct track_shared _track_shared;

      friend class ::boost::serialization::access;
      template <typename Archive>
      void serialize (Archive & ar, unsigned int)
      {
        ar & name_;
        ar & data_;
        ar & condition_;
        ar & _ports_input;
        ar & _ports_output;
        ar & _ports_tunnel;
        ar & port_id_counter_;
        ar & prop_;
        ar & _requirements;
        ar & _preferences;
        ar & _priority;
        ar & eureka_id_;
        ar & _track_shared;
      }
    };
  }

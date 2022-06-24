// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <we/expr/eval/context.hpp>
#include <we/type/Expression.hpp>
#include <we/type/ModuleCall.hpp>
#include <we/type/Port.hpp>
#include <we/type/Requirement.hpp>
#include <we/type/eureka.hpp>
#include <we/type/id.hpp>
#include <we/type/net.fwd.hpp>
#include <we/type/property.hpp>
#include <we/type/value.hpp>

#include <boost/optional.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/variant.hpp>
#include <boost/variant.hpp>
#include <boost/variant/recursive_wrapper.hpp>

#include <unordered_map>
#include <unordered_set>

namespace we
{
  namespace type
  {
    using Preference = std::string;
    using MultiModuleCall = std::unordered_map <Preference, ModuleCall>;

    struct Transition
    {
    private:
      using data_type = ::boost::variant
                      < ModuleCall
                      , MultiModuleCall
                      , Expression
                      , ::boost::recursive_wrapper<we::type::net_type>
                      >
        ;

    public:
      using PortByID = std::unordered_map<we::port_id_type, Port>;

      Transition();

      template <typename Type>
      Transition ( std::string const& name
                   , Type const& typ
                   , ::boost::optional<Expression> const& _condition
                   , we::type::property::type const& prop
                   , we::priority_type priority
                   , ::boost::optional<eureka_id_type> const& _eureka_id
                   , std::list<we::type::Preference> const& preferences
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
      we::type::net_type& mutable_net();

      ::boost::optional<Expression const&> expression() const;
      ::boost::optional<we::type::net_type const&> net() const;
      ::boost::optional<ModuleCall const&> module_call() const;

      ::boost::optional<Expression> const& condition() const;
      ::boost::optional<eureka_id_type> const& eureka_id() const;

      we::port_id_type add_port (Port const&);

      we::port_id_type input_port_by_name (std::string const&) const;
      we::port_id_type const& output_port_by_name (std::string const&) const;

      PortByID const& ports_input() const;
      PortByID const& ports_output() const;
      PortByID const& ports_tunnel() const;

      we::type::property::type const& prop() const;

      std::list<we::type::Requirement> const& requirements() const;
      std::list<we::type::Preference> const& preferences() const;
      void add_requirement (we::type::Requirement const&);

      we::priority_type priority() const;

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
      ::boost::optional<Expression> condition_;

      PortByID _ports_input;
      PortByID _ports_output;
      PortByID _ports_tunnel;
      we::port_id_type port_id_counter_;

      we::type::property::type prop_;

      std::list<we::type::Requirement> _requirements;
      std::list<we::type::Preference> _preferences;
      we::priority_type _priority;

      ::boost::optional<eureka_id_type> eureka_id_;

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
      }
    };
  }
}

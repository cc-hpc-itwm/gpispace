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

#include <we/type/Transition.hpp>

#include <we/exception.hpp>
#include <we/expr/type/AssignResult.hpp>
#include <we/expr/type/Type.hpp>
#include <we/type/net.hpp>
#include <we/type/signature.hpp>
#include <we/type/value/name.hpp>
#include <we/type/value/peek.hpp>
#include <we/type/value/show.hpp>

#include <util-generic/functor_visitor.hpp>
#include <util-generic/join.hpp>
#include <util-generic/nest_exceptions.hpp>
#include <util-generic/print_container.hpp>
#include <util-generic/unreachable.hpp>

#include <boost/format.hpp>
#include <boost/range/adaptor/map.hpp>

#include <algorithm>
#include <stdexcept>
#include <string>

namespace we
{
  namespace type
  {
    namespace
    {
      struct visitor_signature_to_type : ::boost::static_visitor<::expr::Type>
      {
        ::expr::Type operator()
          (std::string const& name) const
        {
#define LITERAL(S,T)                            \
          if (name == ::pnet::type::value::S()) \
          {                                     \
            return ::expr::type::T{};           \
          }

          LITERAL (CONTROL, Control)
          LITERAL (BOOL, Boolean)
          LITERAL (INT, Int)
          LITERAL (LONG, Long)
          LITERAL (UINT, UInt)
          LITERAL (ULONG, ULong)
          LITERAL (FLOAT, Float)
          LITERAL (DOUBLE, Double)
          LITERAL (CHAR, Char)
          LITERAL (STRING, String)
          LITERAL (BITSET, Bitset)
          LITERAL (BYTEARRAY, Bytearray)

#undef LITERAL

#define CONTAINER(S,T,Ts...)                                  \
          if (name == ::pnet::type::value::S())               \
          {                                                   \
            return ::expr::type::T {Ts};                      \
          }

          CONTAINER (LIST, List, expr::type::Any())
          CONTAINER (SET, Set, expr::type::Any())
          CONTAINER (MAP, Map, expr::type::Any(), expr::type::Any())

#undef CONTAINER

          FHG_UTIL_UNREACHABLE();
        }

        ::expr::Type operator()
            (::pnet::type::signature::structured_type const& sig_struct) const
          {
            // auto const& name (sig_struct.first); // ignored
            auto const& fields (sig_struct.second);

            ::expr::type::Struct::Fields field_types;

            for (auto const& field : fields)
            {
              fhg::util::visit<void>
                ( field
                , [&] (std::pair<std::string, std::string> const& f)
                  {
                    field_types.emplace_back
                      ( f.first
                      , this->operator() (f.second)
                      );
                  }
                , [&] (::pnet::type::signature::structured_type const& f)
                  {
                    field_types.emplace_back
                      ( f.first
                      , this->operator() (f)
                      );
                  }
                );
            }

            return ::expr::type::Struct (field_types);
          }
      };

      ::expr::Type signature_to_type
        (::pnet::type::signature::signature_type const& signature)
      {
        return ::boost::apply_visitor (visitor_signature_to_type{}, signature);
      }
    }

    Transition::Transition()
      : name_ ("<<transition unknown>>")
      , data_ (Expression())
      , condition_ (::boost::none)
      , _ports_input()
      , _ports_output()
      , _ports_tunnel()
      , port_id_counter_ (0)
      , prop_()
      , _requirements()
      , _preferences()
      , _priority()
      , eureka_id_ (::boost::none)
    {}

    namespace
    {
      template<typename T, typename Variant>
        ::boost::optional<T const&> get_or_none (Variant const& variant)
      {
        using Ret = ::boost::optional<T const&>;
        return fhg::util::visit<Ret>
          ( variant
          , [] (T const& x) -> Ret { return x; }
          , [] (auto const&) -> Ret { return {}; }
          );
      }
    }

    ::boost::optional<Expression const&> Transition::expression() const
    {
      return get_or_none<Expression> (data_);
    }
    ::boost::optional<we::type::net_type const&> Transition::net() const
    {
      return get_or_none<we::type::net_type> (data_);
    }
    ::boost::optional<ModuleCall const&> Transition::module_call() const
    {
      return get_or_none<ModuleCall> (data_);
    }

    ::boost::optional<Expression> const& Transition::condition() const
    {
      return condition_;
    }

    ::boost::optional<eureka_id_type> const& Transition::eureka_id() const
    {
      return eureka_id_;
    }

    std::string const& Transition::name() const
    {
      return name_;
    }

    Transition::data_type const& Transition::data() const
    {
      return data_;
    }
    we::type::net_type& Transition::mutable_net()
    {
      return ::boost::get<we::type::net_type> (data_);
    }

    std::list<we::type::Requirement> const& Transition::requirements() const
    {
      return _requirements;
    }

    std::list<we::type::Preference> const& Transition::preferences() const
    {
      return _preferences;
    }

    we::port_id_type Transition::add_port (Port const& port)
    {
      auto const port_id (port_id_counter_++);

      fhg::util::visit<void>
        ( port.direction()
        , [&] (port::direction::In const&)
          {
            _ports_input.emplace (port_id, port);
          }
        , [&] (port::direction::Out const&)
          {
            if (net() && !port.associated_place())
            {
              throw std::runtime_error
                ( ( ::boost::format ("Error when adding output port '%1%' to net '%2%':"
                                  " Not associated with any place"
                                  ) % port.name() % name()
                  ).str()
                );
            }
            _ports_output.emplace (port_id, port);
          }
        , [&] (port::direction::Tunnel const&)
          {
            _ports_tunnel.emplace (port_id, port);
          }
        );

      return port_id;
    }

    we::port_id_type Transition::input_port_by_name
      (std::string const& port_name) const
    {
      for (PortByID::value_type const& p : _ports_input)
      {
        if (p.second.name() == port_name)
        {
          return p.first;
        }
      }
      throw pnet::exception::port::unknown (name(), port_name);
    }

    we::port_id_type const& Transition::output_port_by_name
      (std::string const& port_name) const
    {
      for (PortByID::value_type const& p : _ports_output)
      {
        if (p.second.name() == port_name)
        {
          return p.first;
        }
      }
      throw pnet::exception::port::unknown (name(), port_name);
    }

    we::type::property::type const& Transition::prop() const
    {
      return prop_;
    }

    Transition::PortByID const& Transition::ports_input() const
    {
      return _ports_input;
    }
    Transition::PortByID const& Transition::ports_output() const
    {
      return _ports_output;
    }
    Transition::PortByID const& Transition::ports_tunnel() const
    {
      return _ports_tunnel;
    }

    void Transition::add_requirement (we::type::Requirement const& r)
    {
      _requirements.push_back (r);
    }

    we::priority_type Transition::priority() const
    {
      return _priority;
    }

    Transition& Transition::assert_correct_expression_types()
    {
      auto const bind
        ( [] (auto& context, auto const& ports)
          {
            for (auto const& port : ports | ::boost::adaptors::map_values)
            {
              context.bind (port.name(), signature_to_type (port.signature()));
            }
          }
        );

      auto const inference_context_before_eval
        ( [&]
          {
            expr::type::Context context;

            bind (context, _ports_input);
            bind (context, _ports_tunnel);

            return context;
          }
        );
      auto const inference_context_after_eval
        ( [&]
          {
            auto context (inference_context_before_eval());

            bind (context, _ports_output);

            return context;
          }
        );

      auto const require_expression_has_type
        ( [] ( Expression const& expression
             , expr::Type const& expected
             , expr::type::Context context
             , ::boost::format message
             )
          {
            fhg::util::nest_exceptions<std::runtime_error>
              ( [&]
                {
                  expression.assert_type (expected, context);
                }
              , str (message)
              );
          }
        );

      if (condition_)
      {
        require_expression_has_type
          ( *condition_
          , expr::type::Boolean{}
          , inference_context_before_eval()
          , ::boost::format ("In the <condition> expression '%1%'")
          % condition_->expression()
          );

        //! \note this is the reason for the method being non-const
        if (condition_->ast().is_const_true())
        {
          condition_ = ::boost::none;
        }
      }

      if (eureka_id_)
      {
        require_expression_has_type
          ( *eureka_id_
          , expr::type::String()
          , inference_context_before_eval()
          , ::boost::format ("In the <eureka-group> expression '%1%'")
          % *eureka_id_
          );
      }

      auto const require_property_expression_has_type_if_present
        ( [&] ( property::path_type const& path
              , expr::Type const& expected
              , expr::type::Context context
              )
          {
            if (auto property = prop_.get (path))
            {
              if (!::boost::get<std::string> (&*property))
              {
                throw std::runtime_error
                  (str ( ::boost::format
                           ("In the property at '%1%': '%2%' is not a string."
                           " The property at '%1%' must contain a string to be interpreted as an expression."
                           " Did you mean '\"%2%\"'?"
                           )
                       % fhg::util::join (path, ".")
                       % pnet::type::value::show (*property)
                       )
                  );
              }

              require_expression_has_type
                ( ::boost::get<std::string> (*property)
                , expected
                , context
                , ::boost::format ("In the property at '%1%'")
                % fhg::util::join (path, ".")
                );
            }
          }
        );

      //! \note exploits internal knowledge of `we::activity_t`
      require_property_expression_has_type_if_present
        ( {"fhg", "drts", "schedule", "num_worker"}
        , expr::type::ULong()
        , inference_context_before_eval()
        );

      //! \note exploits internal knowledge of `we::activity_t`
      require_property_expression_has_type_if_present
        ( {"fhg", "drts", "schedule", "maximum_number_of_retries"}
        , expr::type::ULong()
        , inference_context_before_eval()
        );

      //! \note exploits internal knowledge of `we::activity_t`
      require_property_expression_has_type_if_present
        ( {"fhg", "drts", "require", "dynamic_requirement"}
        , expr::type::String()
        , inference_context_before_eval()
        );

      auto const assert_outputs_have_correct_type
        ( [&] (auto const& context)
          {
            for (auto const& port : _ports_output | ::boost::adaptors::map_values)
            {
              auto const mtype (context.at (port.name()));
              auto const port_type (signature_to_type (port.signature()));

              if (!mtype)
              {
                throw pnet::exception::missing_binding (port.name());
              }

              fhg::util::nest_exceptions<std::runtime_error>
                ( [&]
                  {
                    if (  port_type
                       != assign_result (port.name(), port_type, *mtype)
                       )
                    {
                      // std::clog
                      //   << ( ::boost::format
                      //        ("On output port '%1%':"
                      //        " The declared type '%2%' is different from the"
                      //        " infered type '%3%'"
                      //        )
                      //      % port.name()
                      //      % port_type
                      //      % *mtype
                      //      )
                      //   << std::endl
                      //   ;
                      (void) port_type;
                    }
                  }
                , str ( ::boost::format ("Output port '%1%' expects type '%2%'")
                      % port.name()
                      % port_type
                      )
                );
            }
          }
        )
        ;

      auto const require_context_key_with_type
        ( [] (auto key, expr::Type expected, auto context)
          {
            auto const type (context.at ({key}));

            if (!type)
            {
              throw pnet::exception::missing_binding (key);
            }

            if (*type != expected)
            {
              throw pnet::exception::type_error
                (str ( ::boost::format
                         ("'%1%' has type '%2%' but expected is type '%3%'")
                     % key
                     % *type
                     % expected
                     )
                );
            }
          }
        );

      fhg::util::visit<void>
        ( data_
        , [&] (ModuleCall& m)
          {
            m.assert_correct_expression_types
              ( inference_context_before_eval()
              , inference_context_after_eval()
              );
          }
        , [&] (MultiModuleCall& ms)
          {
            for (auto const& m : ms)
            {
              fhg::util::nest_exceptions<std::runtime_error>
                ( [&]
                  {
                    m.second.assert_correct_expression_types
                      ( inference_context_before_eval()
                      , inference_context_after_eval()
                      );
                  }
                , str ( ::boost::format ("In the <preference> of module '%1%'")
                      % m.first
                      )
                );
            }
          }
        , [] (net_type& net)
          {
            net.assert_correct_expression_types();
          }
        , [&] (Expression& e)
          {
            fhg::util::nest_exceptions<pnet::exception::type_error>
              ( [&]
                {
                  auto context (inference_context_before_eval());

                  (void) e.type (context);

                  //! \note exploits internal knowledge of `we::net_type`
                  if (prop_.get ({"gspc", "we", "plugin", "create"}))
                  {
                    context.bind ({"plugin_id"}, expr::type::ULong{});
                  }

                  assert_outputs_have_correct_type (context);
                }
              , str (::boost::format ("In expression '%1%'") % e)
              );

            //! \note exploits internal knowledge of `we::net_type`
            if (prop_.get ({"gspc", "we", "plugin", "create"}))
            {
              require_context_key_with_type
                ( "plugin_path"
                , expr::type::String{}
                , inference_context_after_eval()
                );
            }

            //! \note exploits internal knowledge of `we::net_type`
            if (prop_.get ({"gspc", "we", "plugin", "destroy"}))
            {
              require_context_key_with_type
                ( "plugin_id"
                , expr::type::ULong{}
                , inference_context_before_eval()
                );
            }

            //! \note exploits internal knowledge of `we::net_type`
            require_property_expression_has_type_if_present
              ( {"gspc", "we", "plugin", "call_before_eval"}
              , expr::type::List (expr::type::ULong{})
              , inference_context_before_eval()
              );

            //! \note exploits internal knowledge of `we::net_type`
            require_property_expression_has_type_if_present
              ( {"gspc", "we", "plugin", "call_after_eval"}
              , expr::type::List (expr::type::ULong{})
              , inference_context_after_eval()
              );
          }
        );

      return *this;
    }

    bool Transition::might_use_virtual_memory() const
    {
      return fhg::util::visit<bool>
        ( data_
        , [&] (we::type::net_type const& net)
          {
            return net.might_use_virtual_memory();
          }
        , [&] (ModuleCall const& module)
          {
            return !module.memory_buffers().empty();
          }
        , [&] (MultiModuleCall const& multimodule)
          {
            return std::any_of
              ( multimodule.begin()
              , multimodule.end()
              , [&] (auto const& preference_and_module)
                {
                  return !preference_and_module.second.memory_buffers().empty();
                }
              );
          }
        , [&] (Expression const&)
          {
            return false;
          }
        );
    }

    bool Transition::might_have_tasks_requiring_multiple_workers() const
    {
      return fhg::util::visit<bool>
        ( data_
        , [&] (we::type::net_type const& net)
          {
            return net.might_have_tasks_requiring_multiple_workers();
          }
        , [&] (ModuleCall const&)
          {
            return !!prop_.get
              ({"fhg", "drts", "schedule", "num_worker"});
          }
        , [&] (MultiModuleCall const&)
          {
            return !!prop_.get
              ({"fhg", "drts", "schedule", "num_worker"});
          }
        , [&] (Expression const&)
          {
            return false;
          }
        );
    }

    bool Transition::might_use_modules_with_multiple_implementations() const
    {
      return fhg::util::visit<bool>
        ( data_
        , [&] (we::type::net_type const& net)
          {
            return net.might_use_modules_with_multiple_implementations();
          }
        , [&] (ModuleCall const&)
          {
            return false;
          }
        , [&] (MultiModuleCall const&)
          {
            return true;
          }
        , [&] (Expression const&)
          {
            return false;
          }
        );
    }
  }
}

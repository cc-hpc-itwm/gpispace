// Copyright (C) 2013-2014,2016,2019-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/we/type/Transition.hpp>

#include <gspc/we/exception.hpp>
#include <gspc/we/expr/type/AssignResult.hpp>
#include <gspc/we/expr/type/Type.hpp>
#include <gspc/we/type/net.hpp>
#include <gspc/we/type/shared.hpp>
#include <gspc/we/type/signature.hpp>
#include <gspc/we/type/value/name.hpp>
#include <gspc/we/type/value/peek.hpp>
#include <gspc/we/type/value/show.hpp>

#include <gspc/util/functor_visitor.hpp>
#include <gspc/util/join.hpp>
#include <gspc/util/print_container.hpp>
#include <gspc/util/unreachable.hpp>

#include <gspc/util/fmt/boost/variant.formatter.hpp>
#include <gspc/util/join.formatter.hpp>
#include <gspc/we/expr/type/Type.formatter.hpp>
#include <gspc/we/type/Expression.formatter.hpp>
#include <gspc/we/type/value/show.formatter.hpp>
#include <algorithm>
#include <fmt/core.h>
#include <stdexcept>
#include <string>


  namespace gspc::we::type
  {
    namespace
    {
      struct visitor_signature_to_type
        : private ::boost::static_visitor<expr::Type>
      {
        expr::Type operator()
          (std::string const& name) const
        {
#define LITERAL(S,T)                            \
          if (name == pnet::type::value::S()) \
          {                                     \
            return expr::type::T{};           \
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
          LITERAL (BIGINT, Bigint)

#undef LITERAL

          // Handle shared_PLACENAME types
          //
          if (auto cleanup_place {shared::cleanup_place (name)})
          {
            return expr::type::Shared {std::move (*cleanup_place)};
          }

#define CONTAINER(S,T,Ts...)                                  \
          if (name == pnet::type::value::S())               \
          {                                                   \
            return expr::type::T {Ts};                      \
          }

          CONTAINER (LIST, List, expr::type::Any())
          CONTAINER (SET, Set, expr::type::Any())
          CONTAINER (MAP, Map, expr::type::Any(), expr::type::Any())

#undef CONTAINER

          FHG_UTIL_UNREACHABLE();
        }

        expr::Type operator()
            (pnet::type::signature::structured_type const& sig_struct) const
          {
            // auto const& name (sig_struct.first); // ignored
            auto const& fields (sig_struct.second);

            expr::type::Struct::Fields field_types;

            for (auto const& field : fields)
            {
              util::visit<void>
                ( field
                , [&] (std::pair<std::string, std::string> const& f)
                  {
                    field_types.emplace_back
                      ( f.first
                      , this->operator() (f.second)
                      );
                  }
                , [&] (pnet::type::signature::structured_type const& f)
                  {
                    field_types.emplace_back
                      ( f.first
                      , this->operator() (f)
                      );
                  }
                );
            }

            return expr::type::Struct (field_types);
          }
      };

      expr::Type signature_to_type
        (pnet::type::signature::signature_type const& signature)
      {
        return ::boost::apply_visitor (visitor_signature_to_type{}, signature);
      }
    }

    Transition::Transition()
      : name_ ("<<transition unknown>>")
      , data_ (Expression())
      , condition_ (std::nullopt)
      , _ports_input()
      , _ports_output()
      , _ports_tunnel()
      , port_id_counter_ (0)
      , prop_()
      , _requirements()
      , _preferences()
      , _priority()
      , eureka_id_ (std::nullopt)
    {}

    namespace
    {
      template<typename T, typename Variant>
        std::optional<std::reference_wrapper<T const>> get_or_none (Variant const& variant)
      {
        using Ret = std::optional<std::reference_wrapper<T const>>;
        return util::visit<Ret>
          ( variant
          , [] (T const& x) -> Ret { return std::cref(x); }
          , [] (auto const&) -> Ret { return {}; }
          );
      }
    }

    std::optional<std::reference_wrapper<Expression const>> Transition::expression() const
    {
      return get_or_none<Expression> (data_);
    }
    std::optional<std::reference_wrapper<net_type const>> Transition::net() const
    {
      return get_or_none<net_type> (data_);
    }
    std::optional<std::reference_wrapper<ModuleCall const>> Transition::module_call() const
    {
      return get_or_none<ModuleCall> (data_);
    }

    std::optional<Expression> const& Transition::condition() const
    {
      return condition_;
    }

    std::optional<eureka_id_type> const& Transition::eureka_id() const
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
    net_type& Transition::mutable_net()
    {
      return ::boost::get<net_type> (data_);
    }

    std::list<Requirement> const& Transition::requirements() const
    {
      return _requirements;
    }

    std::list<Preference> const& Transition::preferences() const
    {
      return _preferences;
    }

    we::port_id_type Transition::add_port (Port const& port)
    {
      auto const port_id (port_id_counter_++);

      util::visit<void>
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
                { fmt::format ( "Error when adding output port '{}' to net '{}':"
                                " Not associated with any place"
                              , port.name()
                              , name()
                              )
                };
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

    property::type const& Transition::prop() const
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

    void Transition::add_requirement (Requirement const& r)
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
            for (auto const& [_ignore, port] : ports)
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
             , std::string message
             )
          {
            try
            {
              expression.assert_type (expected, context);
            }
            catch (...)
            {
              std::throw_with_nested (std::runtime_error {message});
            }
          }
        );

      if (condition_)
      {
        require_expression_has_type
          ( *condition_
          , expr::type::Boolean{}
          , inference_context_before_eval()
          , fmt::format ( "In the <condition> expression '{}'"
                        , condition_->expression()
                        )
            );

        //! \note this is the reason for the method being non-const
        if (condition_->ast().is_const_true())
        {
          condition_ = std::nullopt;
        }
      }

      if (eureka_id_)
      {
        require_expression_has_type
          ( *eureka_id_
          , expr::type::String()
          , inference_context_before_eval()
          , fmt::format ( "In the <eureka-group> expression '{}'"
                        , *eureka_id_
                        )
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
              if (!::boost::get<std::string> (&property->get()))
              {
                throw std::runtime_error
                  { fmt::format
                      ( "In the property at '{0}': '{1}' is not a string."
                        " The property at '{0}' must contain a string to be interpreted as an expression."
                        " Did you mean '\"{1}\"'?"
                      , util::join (path, ".")
                      , pnet::type::value::show (property->get())
                      )
                  };
              }

              require_expression_has_type
                ( ::boost::get<std::string> (property->get())
                , expected
                , context
                , fmt::format ( "In the property at '{}'"
                              , util::join (path, ".")
                              )
                );
            }
          }
        );

      //! \note exploits internal knowledge of `gspc::we::activity_t`
      require_property_expression_has_type_if_present
        ( {"fhg", "drts", "schedule", "num_worker"}
        , expr::type::ULong()
        , inference_context_before_eval()
        );

      //! \note exploits internal knowledge of `gspc::we::activity_t`
      require_property_expression_has_type_if_present
        ( {"fhg", "drts", "schedule", "maximum_number_of_retries"}
        , expr::type::ULong()
        , inference_context_before_eval()
        );

      //! \note exploits internal knowledge of `gspc::we::activity_t`
      require_property_expression_has_type_if_present
        ( {"fhg", "drts", "require", "dynamic_requirement"}
        , expr::type::String()
        , inference_context_before_eval()
        );

      //! \note exploits internal knowledge of `gspc::scheduler::daemon::NotificationEvent`
      require_property_expression_has_type_if_present
        ( {"gspc", "monitor", "color", "started"}
        , expr::type::Int()
        , inference_context_before_eval()
        );

      //! \note exploits internal knowledge of `gspc::scheduler::daemon::NotificationEvent`
      require_property_expression_has_type_if_present
        ( {"gspc", "monitor", "color", "finished"}
        , expr::type::Int()
        , inference_context_before_eval()
        );

      //! \note exploits internal knowledge of `gspc::scheduler::daemon::NotificationEvent`
      require_property_expression_has_type_if_present
        ( {"gspc", "monitor", "color", "failed"}
        , expr::type::Int()
        , inference_context_before_eval()
        );

      //! \note exploits internal knowledge of `gspc::scheduler::daemon::NotificationEvent`
      require_property_expression_has_type_if_present
        ( {"gspc", "monitor", "color", "canceled"}
        , expr::type::Int()
        , inference_context_before_eval()
        );

      auto const assert_outputs_have_correct_type
        ( [&] (auto const& context)
          {
            for (auto const& [_ignore, port] : _ports_output)
            {
              auto const mtype (context.at (port.name()));
              auto const port_type (signature_to_type (port.signature()));

              if (!mtype)
              {
                throw pnet::exception::missing_binding (port.name());
              }

              try
              {
                if (  port_type
                   != assign_result (port.name(), port_type, *mtype)
                   )
                {
                  // std::clog
                  //   << ( fmt::format
                  //        ("On output port '{}':"
                  //        " The declared type '{}' is different from the"
                  //        " infered type '{}'"
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
              catch (...)
              {
                std::throw_with_nested
                  ( std::runtime_error
                    { fmt::format ( "Output port '{}' expects type '{}'"
                                  , port.name()
                                  , port_type
                                  )
                    }
                  );
              }
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
                { fmt::format
                    ( "'{}' has type '{}' but expected is type '{}'"
                    , key
                    , *type
                    , expected
                    )
                };
            }
          }
        );

      util::visit<void>
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
              try
              {
                m.second.assert_correct_expression_types
                  ( inference_context_before_eval()
                  , inference_context_after_eval()
                  );
              }
              catch (...)
              {
                std::throw_with_nested
                  ( std::runtime_error
                    { fmt::format ( "In the <preference> of module '{}'"
                                  , m.first
                                  )
                    }
                  );
              }
            }
          }
        , [] (net_type& net)
          {
            net.assert_correct_expression_types();
          }
        , [&] (Expression& e)
          {
            try
            {
              auto context (inference_context_before_eval());

              std::ignore = e.type (context);

              //! \note exploits internal knowledge of `gspc::we::net_type`
              if (prop_.get ({"gspc", "we", "plugin", "create"}))
              {
                context.bind ({"plugin_id"}, expr::type::ULong{});
              }

              assert_outputs_have_correct_type (context);
            }
            catch (...)
            {
              std::throw_with_nested
                ( pnet::exception::type_error
                  { fmt::format ("In expression '{}'", e)
                  }
                );
            }

            //! \note exploits internal knowledge of `gspc::we::net_type`
            if (prop_.get ({"gspc", "we", "plugin", "create"}))
            {
              require_context_key_with_type
                ( "plugin_path"
                , expr::type::String{}
                , inference_context_after_eval()
                );
            }

            //! \note exploits internal knowledge of `gspc::we::net_type`
            if (prop_.get ({"gspc", "we", "plugin", "destroy"}))
            {
              require_context_key_with_type
                ( "plugin_id"
                , expr::type::ULong{}
                , inference_context_before_eval()
                );
            }

            //! \note exploits internal knowledge of `gspc::we::net_type`
            require_property_expression_has_type_if_present
              ( {"gspc", "we", "plugin", "call_before_eval"}
              , expr::type::List (expr::type::ULong{})
              , inference_context_before_eval()
              );

            //! \note exploits internal knowledge of `gspc::we::net_type`
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
      return util::visit<bool>
        ( data_
        , [&] (net_type const& net)
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
      return util::visit<bool>
        ( data_
        , [&] (net_type const& net)
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
      return util::visit<bool>
        ( data_
        , [&] (net_type const& net)
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

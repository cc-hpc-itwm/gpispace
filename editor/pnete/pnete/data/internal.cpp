// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/data/internal.hpp>

#include <pnete/data/proxy.hpp>
#include <pnete/weaver/display.hpp>

#include <xml/parse/parser.hpp>
#include <xml/parse/type/expression.hpp>
#include <xml/parse/type/function.hpp>
#include <xml/parse/type/mod.hpp>
#include <xml/parse/type/net.hpp>

#include <QObject>
#include <QString>

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      namespace
      {
        ::xml::parse::id::ref::function make_function
          (const internal_type::kind& kind, ::xml::parse::state::type& state)
        {
          const ::xml::parse::id::function function_id
            (state.id_mapper()->next_id());
          switch (kind)
          {
          case internal_type::expression:
            {
              return ::xml::parse::type::function_type
                ( function_id
                , state.id_mapper()
                , boost::none
                , ::xml::parse::type::expression_type
                  ( state.id_mapper()->next_id()
                  , state.id_mapper()
                  , function_id
                  ).make_reference_id()
                ).make_reference_id();
            }
          case internal_type::module_call:
            {
              return ::xml::parse::type::function_type
                ( function_id
                , state.id_mapper()
                , boost::none
                , ::xml::parse::type::module_type
                  ( state.id_mapper()->next_id()
                  , state.id_mapper()
                  , function_id
                  ).make_reference_id()
                ).make_reference_id();
            }
          case internal_type::net:
            {
              return ::xml::parse::type::function_type
                ( function_id
                , state.id_mapper()
                , boost::none
                , ::xml::parse::type::net_type
                  ( state.id_mapper()->next_id()
                  , state.id_mapper()
                  , function_id
                  ).make_reference_id()
                ).make_reference_id();
            }
          }
          throw std::runtime_error ("make_function of unknown kind!?");
        }
      }

      internal_type::internal_type (const kind& kind_)
        : _state ()
        , _function (make_function (kind_, _state))
        , _change_manager (NULL)
        , _root_proxy (weaver::display::function (_function, this))
      {}

      internal_type::internal_type (const QString& filename)
        : _state ()
        , _function ( ::xml::parse::just_parse ( _state
                                               , filename.toStdString()
                                               )
                    )
        , _change_manager (NULL)
        , _root_proxy (weaver::display::function (_function, this))
      {}

      const ::xml::parse::id::ref::function& internal_type::function() const
      {
        return _function;
      }

      const ::xml::parse::state::key_values_t & internal_type::context () const
      {
        return _state.key_values();
      }
      const ::xml::parse::state::type & internal_type::state () const
      {
        return _state;
      }
      ::xml::parse::state::type & internal_type::state ()
      {
        return _state;
      }
      change_manager_t& internal_type::change_manager ()
      {
        return _change_manager;
      }
      const proxy::type& internal_type::root_proxy() const
      {
        return _root_proxy;
      }
      proxy::type& internal_type::root_proxy()
      {
        return _root_proxy;
      }
    }
  }
}

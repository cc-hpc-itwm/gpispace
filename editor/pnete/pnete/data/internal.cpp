// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/data/internal.hpp>

#include <xml/parse/parser.hpp>

#include <QObject>
#include <QString>

#include <pnete/weaver/display.hpp>
#include <pnete/data/proxy.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      namespace
      {
        ::xml::parse::id::function make_function
          (const internal_type::kind& kind, ::xml::parse::state::type& state)
        {
          const ::xml::parse::id::function function_id (state.next_id());
          switch (kind)
          {
          case internal_type::expression:
            {
              const ::xml::parse::id::expression id (state.next_id());

              const ::xml::parse::type::expression_type expression
                (id, state.id_mapper(), function_id);

              const ::xml::parse::type::function_type fun
                ( function_id
                , state.id_mapper()
                , ::xml::parse::id::ref::expression (id, state.id_mapper())
                , boost::none
                );
              break;
            }
          case internal_type::module_call:
            {
              const ::xml::parse::id::module id (state.next_id());

              const ::xml::parse::type::module_type mod
                (id, state.id_mapper(), function_id);

              const ::xml::parse::type::function_type fun
                ( function_id
                , state.id_mapper()
                , ::xml::parse::id::ref::module (id, state.id_mapper())
                , boost::none
                );
              break;
            }
          case internal_type::net:
            {
              const ::xml::parse::id::net id (state.next_id());

              const ::xml::parse::type::net_type net
                (id, state.id_mapper(), function_id);

              const ::xml::parse::type::function_type fun
                ( function_id
                , state.id_mapper()
                , ::xml::parse::id::ref::net (id, state.id_mapper())
                , boost::none
                );
              break;
            }
          default:
            {
              throw std::runtime_error ("make_function of unknown kind!?");
            }
          }
          return function_id;
        }
      }

      internal_type::internal_type (const kind& kind_)
        : _state ()
        , _function (make_function (kind_, _state), _state.id_mapper())
        , _change_manager (_state)
        , _root_proxy (*create_proxy())
      {}

      internal_type::internal_type (const QString& filename)
        : _state ()
        , _function ( ::xml::parse::just_parse ( _state
                                               , filename.toStdString()
                                               )
                    )
        , _change_manager (_state)
        , _root_proxy (*create_proxy())
      {}

      proxy::type* internal_type::create_proxy()
      {
        return weaver::function
          (weaver::function_with_mapping_type (function()), this).proxy();
      }

      ::xml::parse::type::function_type & internal_type::function ()
      {
        return _function.get_ref();
      }
      const ::xml::parse::type::function_type & internal_type::function () const
      {
        return _function.get();
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

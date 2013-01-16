// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/data/internal.hpp>

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
        ::xml::parse::type::function_type::content_type make_function_content
          (const internal_type::kind& kind, ::xml::parse::state::type& state)
        {
          switch (kind)
          {
          case internal_type::expression:
            {
              return ::xml::parse::type::expression_type
                ( state.id_mapper()->next_id()
                , state.id_mapper()
                , boost::none
                ).make_reference_id();
            }
          case internal_type::module_call:
            {
              return ::xml::parse::type::module_type
                ( state.id_mapper()->next_id()
                , state.id_mapper()
                , boost::none
                ).make_reference_id();
            }
          case internal_type::net:
            {
              return ::xml::parse::type::net_type
                ( state.id_mapper()->next_id()
                , state.id_mapper()
                , boost::none
                ).make_reference_id();
            }
          }
          throw std::runtime_error ("make_function_content of unknown kind!?");
        }
      }

      internal_type::internal_type (const kind& kind_)
        : _state()
        , _function ( ::xml::parse::type::function_type
                      ( _state.id_mapper()->next_id()
                      , _state.id_mapper()
                      , boost::none
                      , make_function_content (kind_, _state)
                      ).make_reference_id()
                    )
        , _change_manager (NULL)
      {}

      internal_type::internal_type (const QString& filename)
        : _state()
        , _function ( ::xml::parse::just_parse ( _state
                                               , filename.toStdString()
                                               )
                    )
        , _change_manager (NULL)
      {}

      const ::xml::parse::id::ref::function& internal_type::function() const
      {
        return _function;
      }
      change_manager_t& internal_type::change_manager()
      {
        return _change_manager;
      }
    }
  }
}

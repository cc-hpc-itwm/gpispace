// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/data/internal.hpp>

#include <xml/parse/parser.hpp>
#include <xml/parse/type/expression.hpp>
#include <xml/parse/type/function.hpp>
#include <xml/parse/type/mod.hpp>
#include <xml/parse/type/net.hpp>
#include <xml/parse/type/link.hpp>

#include <xml/parse/util/position.hpp>

#include <QObject>
#include <QString>
#include <QSettings>

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
                , XML_PARSE_UTIL_POSITION_GENERATED()
                ).make_reference_id();
            }
          case internal_type::module_call:
            {
              return ::xml::parse::type::module_type
                ( state.id_mapper()->next_id()
                , state.id_mapper()
                , boost::none
                , XML_PARSE_UTIL_POSITION_GENERATED()
                ).make_reference_id();
            }
          case internal_type::net:
            {
              return ::xml::parse::type::net_type
                ( state.id_mapper()->next_id()
                , state.id_mapper()
                , boost::none
                , XML_PARSE_UTIL_POSITION_GENERATED()
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
                      , XML_PARSE_UTIL_POSITION_GENERATED()
                      , make_function_content (kind_, _state)
                      ).make_reference_id()
                    )
        , _change_manager (nullptr)
      {}

      namespace
      {
        std::vector<std::string> get_includes_from_settings()
        {
          std::vector<std::string> includes;

          QSettings settings;

          const int size (settings.beginReadArray ("template_includes"));

          for (int i (0); i < size; ++i)
          {
            settings.setArrayIndex (i);

            includes.emplace_back
              (settings.value ("path").toString().toStdString());
          }

          settings.endArray();

          return includes;
        }
      }

      internal_type::internal_type (const QString& filename)
        : _state (get_includes_from_settings())
        , _function ( ::xml::parse::just_parse ( _state
                                               , filename.toStdString()
                                               )
                    )
        , _change_manager (nullptr)
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

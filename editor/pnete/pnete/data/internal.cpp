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
      namespace internal
      {
        namespace detail
        {
          static ::xml::parse::type::function_type::type make_function
            (const type::kind& t)
          {
            switch (t)
              {
              case type::expression: return ::xml::parse::type::expression_type();
              case type::module_call: return ::xml::parse::type::mod_type();
              case type::net: return ::xml::parse::type::net_type();
              default: throw std::runtime_error ("make_function of unknown kind!?");
              }
          }
        }

        type::type (const kind& kind_)
          : _state ()
          , _function (detail::make_function (kind_))
          , _change_manager (*this)
          , _root_proxy (*create_proxy())
        {}

        type::type (const QString& filename)
          : _state ()
          , _function (::xml::parse::just_parse ( _state
                                                , filename.toStdString()
                                                )
                      )
          , _change_manager (*this)
          , _root_proxy (*create_proxy())
        {}

        proxy::type* type::create_proxy()
        {
          return weaver::function
            (weaver::function_with_mapping_type (function()), this).proxy();
        }

        ::xml::parse::type::function_type & type::function ()
        {
          return const_cast< ::xml::parse::type::function_type &> (_function);
        }

        const ::xml::parse::type::function_type & type::function () const
        {
          return _function;
        }
        const ::xml::parse::state::key_values_t & type::context () const
        {
          return _state.key_values();
        }
        const ::xml::parse::state::type & type::state () const
        {
          return _state;
        }
        change_manager_t& type::change_manager ()
        {
          return _change_manager;
        }
        const proxy::type& type::root_proxy() const
        {
          return _root_proxy;
        }
        proxy::type& type::root_proxy()
        {
          return _root_proxy;
        }
      }
    }
  }
}

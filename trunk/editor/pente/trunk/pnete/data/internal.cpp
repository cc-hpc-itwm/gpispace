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
      internal_type::internal_type()
        : _state ()
        , _function ()
        , _change_manager (*this)
        , _root_proxy (*create_proxy())
      {}

      internal_type::internal_type (const QString& filename)
        : _state ()
        , _function (::xml::parse::just_parse (_state, filename.toStdString()))
        , _change_manager (*this)
        , _root_proxy (*create_proxy())
      {}

      proxy::type* internal_type::create_proxy()
      {
        return weaver::function
          (weaver::function_with_mapping_type (function()), this).proxy();
      }

      ::xml::parse::type::function_type & internal_type::function ()
      {
        return const_cast< ::xml::parse::type::function_type &> (_function);
      }

      const ::xml::parse::type::function_type & internal_type::function () const
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

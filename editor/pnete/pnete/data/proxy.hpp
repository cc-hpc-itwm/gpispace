// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef _PNETE_DATA_PROXY_HPP
#define _PNETE_DATA_PROXY_HPP 1

#include <boost/variant.hpp>

#include <xml/parse/type/function.hpp>
#include <xml/parse/type/expression.hpp>
#include <xml/parse/type/mod.hpp>
#include <xml/parse/type/net.hpp>
#include <xml/parse/type/port.hpp>
#include <xml/parse/type_map_type.hpp>

#include <QString>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      class document_view;

      namespace graph
      {
        class scene_type;
      }
    }

    namespace data
    {
      class internal_type;

      namespace proxy
      {
        template<typename DATA, typename DISPLAY = void>
        class proxy_base
        {
        public:
          typedef DATA data_type;
          typedef DISPLAY display_type;

          proxy_base ( internal_type* root
                     , data_type data
                     , const ::xml::parse::id::ref::function& function
                     , display_type* display = NULL
                     )
            : _data (data)
            , _function (function)
            , _display (display)
            , _root (root)
          {}
          data_type& data()
          {
            return _data;
          }
          const ::xml::parse::id::ref::function& function() const
          {
            return _function;
          }
          display_type* display() const
          {
            return _display;
          }
          internal_type* root() const
          {
            return _root;
          }

        private:
          data_type _data;
          ::xml::parse::id::ref::function _function;
          display_type* _display;

          internal_type* _root;
        };

        typedef proxy_base< ::xml::parse::id::ref::expression> expression_proxy;
        typedef proxy_base< ::xml::parse::id::ref::module> mod_proxy;
        typedef proxy_base< ::xml::parse::id::ref::net
                          , ui::graph::scene_type
                          > net_proxy;

        typedef boost::variant<expression_proxy, mod_proxy, net_proxy> type;

        ::xml::parse::id::ref::function function (const type&);
        ::fhg::pnete::data::internal_type* root (const type&);

        ui::document_view* document_view_factory (type&);
      }
    }
  }
}

#endif

// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef _PNETE_DATA_PROXY_HPP
#define _PNETE_DATA_PROXY_HPP 1

#include <xml/parse/id/types.hpp>

#include <boost/variant.hpp>

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
        template<typename data_type, typename display_type = void>
        class proxy_base
        {
        public:
          proxy_base ( internal_type* root
                     , const ::xml::parse::id::ref::function& function
                     , data_type data
                     , display_type* display = NULL
                     )
            : _root (root)
            , _function (function)
            , _data (data)
            , _display (display)
          { }

          internal_type* root() const
          {
            return _root;
          }
          const ::xml::parse::id::ref::function& function() const
          {
            return _function;
          }

          data_type& data()
          {
            return _data;
          }
          display_type* display() const
          {
            return _display;
          }

        private:
          internal_type* _root;
          ::xml::parse::id::ref::function _function;

          data_type _data;
          display_type* _display;
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

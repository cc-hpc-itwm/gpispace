// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/data/handle/function.hpp>

#include <pnete/data/change_manager.hpp>
#include <pnete/data/handle/expression.hpp>
#include <pnete/data/handle/module.hpp>
#include <pnete/data/handle/net.hpp>

#include <xml/parse/type/function.hpp>

#include <boost/variant.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      namespace handle
      {
        function::function ( const function_meta_base::id_type& id
                           , internal_type* document
                           )
          : function_meta_base (id, document)
        { }

        void function::set_property ( const QObject* sender
                                    , const ::we::type::property::key_type& key
                                    , const ::we::type::property::value_type& val
                                    ) const
        {
          change_manager().set_property (sender, *this, key, val);
        }

        void function::set_name (const QObject* sender, const QString& name) const
        {
          change_manager().set_function_name (sender, *this, name);
        }

        void function::add_port ( const QObject* origin
                                , const we::type::PortDirection& direction
                                , const boost::optional<QPointF>& position
                                ) const
        {
          change_manager().add_port (origin, *this, direction, position);
        }

        namespace
        {
          typedef boost::variant<expression, module, net> variant_type;

          class wrap_with_handle : public boost::static_visitor<variant_type>
          {
          public:
            wrap_with_handle (internal_type* document)
              : _document (document)
            { }

#define CASE(NAME)                                                \
            variant_type operator()                               \
              (const ::xml::parse::id::ref::NAME& id) const       \
            {                                                     \
              return NAME (id, _document);                        \
            }

            CASE (expression);
            CASE (module);
            CASE (net);

#undef CASE

          private:
            internal_type* _document;
          };
        }

        variant_type function::content_handle() const
        {
          return boost::apply_visitor
            (wrap_with_handle (document()), get().content());
        }
      }
    }
  }
}

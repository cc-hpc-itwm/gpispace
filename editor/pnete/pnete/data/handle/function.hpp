// bernd.loerwald@itwm.fraunhofer.de

#ifndef _FHG_PNETE_DATA_HANDLE_FUNCTION_HPP
#define _FHG_PNETE_DATA_HANDLE_FUNCTION_HPP 1

#include <pnete/data/handle/function.fwd.hpp>

#include <pnete/data/handle/expression.fwd.hpp>
#include <pnete/data/handle/meta_base.hpp>
#include <pnete/data/handle/module.fwd.hpp>
#include <pnete/data/handle/net.fwd.hpp>
#include <pnete/data/handle/port.hpp>

#include <we/type/port.hpp>

#include <xml/parse/id/types.hpp>
#include <xml/parse/type/function.fwd.hpp>

#include <boost/optional/optional_fwd.hpp>
#include <boost/variant/variant_fwd.hpp>

#include <QList>

class QObject;
class QString;

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      namespace handle
      {
        typedef meta_base < ::xml::parse::id::ref::function
                          , ::xml::parse::type::function_type
                          > function_meta_base;
        class function : public function_meta_base
        {
        public:
          function (const function_meta_base::id_type&, internal_type*);

          virtual void set_property ( const QObject* sender
                                    , const ::we::type::property::key_type&
                                    , const ::we::type::property::value_type&
                                    ) const;

          void set_name (const QObject* sender, const QString& name) const;

          void add_port ( const QObject* origin
                        , const we::type::PortDirection&
                        , const boost::optional<QPointF>&
                        ) const;

          boost::variant<expression, module, net> content_handle() const;
          QList<port> ports (const we::type::PortDirection&) const;

          using function_meta_base::operator==;
        };
      }
    }
  }
}

#endif

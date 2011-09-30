// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_PNETE_UI_PORTS_LIST_WIDGET_HPP
#define _FHG_PNETE_UI_PORTS_LIST_WIDGET_HPP 1

#include <QObject>
#include <QWidget>

class QStringList;
class QWidget;

#include <xml/parse/types.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      class ports_list_widget : public QWidget
      {
        Q_OBJECT;

      public:
        explicit ports_list_widget ( ::xml::parse::type::ports_type& in
                                   , ::xml::parse::type::ports_type& out
                                   , const QStringList& types
                                   , QWidget* parent = NULL
                                   );

      private:
        ::xml::parse::type::ports_type& _in;
        ::xml::parse::type::ports_type& _out;

        const QStringList& _types;
      };
    }
  }
}

#endif

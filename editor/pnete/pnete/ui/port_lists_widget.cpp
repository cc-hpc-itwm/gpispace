// mirko.rahn@itwm.fraunhofer.de

#include <pnete/ui/port_lists_widget.hpp>

#include <QWidget>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QSplitter>

#include <pnete/ui/port_list_widget.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      static QGroupBox* port_list_box
        ( const QString & label
        , ::xml::parse::type::function_type::ports_type & ports
        , const QStringList& types
        )
      {
        QGroupBox* group_box (new QGroupBox (label));
        QVBoxLayout* vbox (new QVBoxLayout (group_box));
        vbox->addWidget (new port_list_widget (ports, types));
        group_box->setLayout (vbox);
        return group_box;
      }

      port_lists_widget::port_lists_widget
        ( ::xml::parse::type::function_type::ports_type& in
        , ::xml::parse::type::function_type::ports_type& out
        , const QStringList& types
        , QWidget* parent
        )
          : QWidget (parent)
          , _in (in)
          , _out (out)
          , _types (types)
      {
        QSplitter* splitter (new QSplitter(Qt::Vertical));

        splitter->addWidget (port_list_box (tr ("in"),  _in, _types));
        splitter->addWidget (port_list_box (tr ("out"), _out, _types));

        QVBoxLayout* vbox (new QVBoxLayout());
        vbox->addWidget (splitter);
        vbox->setContentsMargins (0, 0, 0, 0);
        setLayout(vbox);
      }
    }
  }
}

// mirko.rahn@itwm.fraunhofer.de

#include <pnete/ui/ports_list_widget.hpp>

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
        , data::proxy::xml_type::ports_type & ports
        )
      {
        QGroupBox* group_box (new QGroupBox (label));
        QVBoxLayout* vbox (new QVBoxLayout (group_box));
        vbox->addWidget (new port_list_widget (ports));
        group_box->setLayout (vbox);
        return group_box;
      }

      ports_list_widget::ports_list_widget
        ( data::proxy::xml_type::ports_type& in
        , data::proxy::xml_type::ports_type& out
        , QWidget* parent
        )
          : QWidget (parent)
          , _in (in)
          , _out (out)
      {
        QSplitter* splitter (new QSplitter(Qt::Vertical));
        splitter->addWidget(port_list_box (tr ("in"), _in));
        splitter->addWidget(port_list_box (tr ("out"), _out));

        QVBoxLayout* vbox (new QVBoxLayout());
        vbox->addWidget (splitter);
        vbox->setContentsMargins (0, 0, 0, 0);
        setLayout(vbox);
      }
    }
  }
}


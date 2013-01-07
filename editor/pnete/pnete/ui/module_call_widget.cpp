// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/ui/module_call_widget.hpp>

#include <pnete/data/handle/function.hpp>
#include <pnete/ui/port_lists_widget.hpp>

#include <QWidget>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QSplitter>

// remove me
#include <QTextEdit>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      module_call_widget::module_call_widget
        ( data::proxy::type& proxy
        , const ::xml::parse::id::ref::module& mod
        , const data::handle::function& function
        , const QStringList& types
        , QWidget* parent
        )
          : base_editor_widget (proxy, parent)
      {
        QGroupBox* group_box (new QGroupBox (tr ("module call")));
        QHBoxLayout* group_box_layout (new QHBoxLayout());
        group_box->setLayout (group_box_layout);

        QSplitter* splitter (new QSplitter ());
        splitter->addWidget
          (new port_lists_widget (data::proxy::function (proxy), types));

        QTextEdit* edit (new QTextEdit ());
        edit->setText (QString ("<<module foo>>"));
        splitter->addWidget (edit);

        group_box_layout->addWidget (splitter);

        QHBoxLayout* hbox (new QHBoxLayout ());
        hbox->addWidget (group_box);
        setLayout (hbox);
      }
    }
  }
}

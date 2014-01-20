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
        ( const data::handle::module& mod
        , const data::handle::function& function
        , QWidget* parent
        )
          : QWidget (parent)
      {
        QSplitter* splitter (new QSplitter ());
        splitter->addWidget (new port_lists_widget (function, QStringList()));

        QTextEdit* edit (new QTextEdit ());
        edit->setText (QString ("<<module foo>>"));
        splitter->addWidget (edit);

        QHBoxLayout* hbox (new QHBoxLayout ());
        hbox->addWidget (splitter);
        setLayout (hbox);
      }
    }
  }
}

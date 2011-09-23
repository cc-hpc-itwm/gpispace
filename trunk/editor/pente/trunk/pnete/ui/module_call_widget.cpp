// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <QWidget>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QSplitter>

// remove me
#include <QTextEdit>

#include <pnete/ui/module_call_widget.hpp>
#include <pnete/ui/ports_list_widget.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      module_call_widget::module_call_widget
        ( data::proxy::type& proxy
        , data::proxy::mod_proxy::data_type& mod
        , QWidget* parent
        )
          : base_editor_widget (proxy, parent)
          , _mod (mod)
          , _ports_list (new ports_list_widget (mod.in(), mod.out()))
      {
        QGroupBox* group_box (new QGroupBox (tr ("module call")));
        QHBoxLayout* group_box_layout (new QHBoxLayout());
        group_box->setLayout (group_box_layout);

        QSplitter* splitter (new QSplitter ());
        splitter->addWidget (_ports_list);

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

// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <QWidget>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QSplitter>
#include <QGroupBox>

#include <pnete/ui/expression_widget.hpp>
#include <pnete/ui/ports_list_widget.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      expression_widget::expression_widget
        ( data::proxy::type& proxy
        , data::proxy::expression_proxy::data_type& expression
        , QWidget* parent
        )
          : base_editor_widget (proxy, parent)
          , _expression (expression)
          , _ports_list (new ports_list_widget ( expression.in()
                                               , expression.out()
                                               )
                        )
      {
        QGroupBox* group_box (new QGroupBox (tr ("expression")));
        QHBoxLayout* group_box_layout (new QHBoxLayout());
        group_box->setLayout (group_box_layout);

        QSplitter* splitter (new QSplitter ());
        splitter->addWidget (_ports_list);

        QTextEdit* edit (new QTextEdit ());
        edit->setText (QString (expression.expression().expression().c_str()));
        splitter->addWidget (edit);

        group_box_layout->addWidget (splitter);

        QHBoxLayout* hbox (new QHBoxLayout ());
        hbox->addWidget (group_box);
        setLayout (hbox);
      }
    }
  }
}

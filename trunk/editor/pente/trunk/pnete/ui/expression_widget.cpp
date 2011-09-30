// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QSplitter>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>

#include <pnete/ui/expression_widget.hpp>
#include <pnete/ui/ports_list_widget.hpp>

#include <pnete/data/internal.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      expression_widget::expression_widget
        ( data::proxy::type& proxy
        , data::proxy::expression_proxy::data_type& expression
        , const QStringList& types
        , QWidget* parent
        )
          : base_editor_widget (proxy, parent)
          , _expression (expression)
          , _ports_list (new ports_list_widget ( expression.in()
                                               , expression.out()
                                               , types
                                               )
                        )
      {
        QGroupBox* group_box (new QGroupBox (tr ("expression")));
        QHBoxLayout* group_box_layout (new QHBoxLayout());
        group_box->setLayout (group_box_layout);

        QSplitter* splitter (new QSplitter ());
        splitter->addWidget (_ports_list);

        QWidget* exp_widget (new QWidget ());
        QVBoxLayout* vbox (new QVBoxLayout ());
        exp_widget->setLayout (vbox);

        const fhg::util::maybe<std::string> & name
          (data::proxy::function (proxy).name);

        QLineEdit* name_line_edit
          (new QLineEdit (name ? QString::fromStdString(*name) : ""));
        connect ( name_line_edit
                , SIGNAL (textEdited (const QString&))
                , SLOT (name_changed (const QString&))
                );

        QWidget* name_widget (new QWidget ());
        QFormLayout* name_layout (new QFormLayout ());
        name_layout->addRow (tr ("&Name"), name_line_edit);
        name_widget->setLayout (name_layout);
        name_layout->setContentsMargins (0, 0, 0, 0);

        vbox->addWidget (name_widget);

        QTextEdit* edit (new QTextEdit ());
        edit->setText
          (QString::fromStdString (expression.expression().expression()));
        vbox->addWidget (edit);
        vbox->setContentsMargins (0, 0, 0, 0);

        splitter->addWidget (exp_widget);

        group_box_layout->addWidget (splitter);

        QHBoxLayout* hbox (new QHBoxLayout ());
        hbox->addWidget (group_box);
        setLayout (hbox);
      }

      void expression_widget::name_changed (const QString& name_)
      {
        //! \todo implement in base_editor_widget convenience for access to change_manager, function
        data::proxy::root (proxy())->
          change_manager().set_function_name ( data::proxy::function (proxy())
                                             , name_
                                             )
          ;
      }
    }
  }
}

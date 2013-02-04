// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/ui/expression_widget.hpp>

#include <pnete/data/change_manager.hpp>
#include <pnete/data/handle/function.hpp>
#include <pnete/ui/port_lists_widget.hpp>

#include <xml/parse/type/function.hpp>

#include <util/qt/scoped_signal_block.hpp>

#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QSplitter>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      expression_widget::expression_widget
        ( const data::handle::expression& expression
        , const data::handle::function& function
        , QWidget* parent
        )
          : QWidget (parent)
          , _expression (expression)
          , _function (function)
          , _port_lists (new port_lists_widget (_function, QStringList()))
          , _expression_edit (new QTextEdit())
          , _name_edit (new QLineEdit())
          , _parse_result (new QTextEdit())
      {
        QWidget* name_widget (new QWidget ());
        QFormLayout* name_layout (new QFormLayout ());
        name_layout->addRow (tr ("&Name"), _name_edit);
        name_layout->setContentsMargins (0, 0, 0, 0);
        name_widget->setLayout (name_layout);

        QWidget* exp_widget (new QWidget ());
        QVBoxLayout* vbox (new QVBoxLayout ());
        vbox->setContentsMargins (0, 0, 0, 0);
        exp_widget->setLayout (vbox);

        vbox->addWidget (name_widget);
        vbox->addWidget (_expression_edit);

        _parse_result->setReadOnly (true);
        _parse_result->setFont (QFont ("Courier"));

        QSplitter* exp_splitter (new QSplitter (Qt::Vertical));
        exp_splitter->addWidget (exp_widget);
        exp_splitter->addWidget (_parse_result);

        QSplitter* splitter (new QSplitter ());
        splitter->addWidget (_port_lists);
        splitter->addWidget (exp_splitter);

        QHBoxLayout* hbox (new QHBoxLayout ());
        hbox->addWidget (splitter);
        setLayout (hbox);

        _function.connect_to_change_mgr
          ( this
          , "function_name_changed", "slot_set_function_name"
          , "data::handle::function, QString"
          );
        _expression.connect_to_change_mgr
          ( this
          , "signal_set_expression", "slot_set_expression"
          , "data::handle::expression, QString"
          );
        _expression.connect_to_change_mgr
          ( this
          , "signal_set_expression_parse_result"
          , "slot_set_expression_parse_result"
          , "data::handle::expression, QString"
          );

        connect ( _name_edit
                , SIGNAL (textEdited (QString))
                , SLOT (name_changed (QString))
                );

        connect ( _expression_edit
                , SIGNAL (textChanged ())
                , SLOT (expression_changed ())
                );

        set_name (_function.get().name());
        set_expression (expression.content());
      }

      void expression_widget::slot_set_function_name
        (const data::handle::function& fun, const QString& name)
      {
        if (is_my_function (fun))
        {
          set_name (name);
        }
      }

      void expression_widget::slot_set_expression
        (const data::handle::expression& expression, const QString& text)
      {
        if (is_my_expression (expression))
        {
          set_expression (text);
        }
      }

      void expression_widget::slot_set_expression_parse_result
        (const data::handle::expression& expression, const QString& text)
      {
        if (is_my_expression (expression))
        {
          set_expression_parse_result (text);
        }
      }

      void
      expression_widget::set_expression_parse_result (const QString& result)
      {
        _parse_result->setPlainText (result);
      }
      void
      expression_widget::set_name (const boost::optional<std::string>& name)
      {
        set_name (name ? *name : "");
      }
      void expression_widget::set_name (const std::string& name)
      {
        set_name (QString::fromStdString (name));
      }
      void expression_widget::set_name (const QString& name)
      {
        const util::qt::scoped_signal_block block (_name_edit);
        _name_edit->setText (name);
      }
      void expression_widget::set_expression (const std::string& text)
      {
        set_expression (QString::fromStdString (text));
      }
      void expression_widget::set_expression (const QString& text)
      {
        const util::qt::scoped_signal_block block (_expression_edit);
        _expression_edit->setPlainText (text);
      }
      void expression_widget::name_changed (const QString& name)
      {
        _function.set_name (name);
      }

      void expression_widget::expression_changed ()
      {
        _expression.set_content (_expression_edit->toPlainText());
      }

      bool expression_widget::is_my_function
        (const data::handle::function& f)
      {
        return f == _function;
      }
      bool expression_widget::is_my_expression
        (const data::handle::expression& e)
      {
        return e == _expression;
      }
    }
  }
}

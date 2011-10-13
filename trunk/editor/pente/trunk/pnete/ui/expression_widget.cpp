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
#include <pnete/ui/port_lists_widget.hpp>

#include <pnete/data/internal.hpp>

#include <pnete/util.hpp>

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
          , _port_lists (new port_lists_widget ( expression.in()
                                               , expression.out()
                                               , types
                                               )
                        )
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

        QGroupBox* group_box (new QGroupBox (tr ("expression")));
        QHBoxLayout* group_box_layout (new QHBoxLayout());
        group_box->setLayout (group_box_layout);
        group_box_layout->addWidget (splitter);

        QHBoxLayout* hbox (new QHBoxLayout ());
        hbox->addWidget (group_box);
        setLayout (hbox);

        set_name (data::proxy::function (proxy).name);
        set_expression (expression.expression().expression());

        connect ( &data::proxy::root (proxy)->change_manager()
                , SIGNAL ( signal_set_function_name
                           ( const QObject*
                           , const ::xml::parse::type::function_type&
                           , const QString&
                           )
                         )
                , SLOT ( slot_set_function_name
                         ( const QObject*
                         , const ::xml::parse::type::function_type&
                         , const QString&
                         )
                       )
                );
        connect ( &data::proxy::root (proxy)->change_manager()
                , SIGNAL ( signal_set_expression
                           ( const QObject*
                           , const ::xml::parse::type::expression_type&
                           , const QString&
                           )
                         )
                , SLOT ( slot_set_expression
                         ( const QObject*
                         , const ::xml::parse::type::expression_type&
                         , const QString&
                         )
                       )
                );
        connect ( &data::proxy::root (proxy)->change_manager()
                , SIGNAL ( signal_set_expression_parse_result
                           ( const QObject*
                           , const ::xml::parse::type::expression_type&
                           , const QString&
                           )
                         )
                , SLOT ( slot_set_expression_parse_result
                         ( const QObject*
                         , const ::xml::parse::type::expression_type&
                         , const QString&
                         )
                       )
                );

        connect ( _name_edit
                , SIGNAL (textEdited (const QString&))
                , SLOT (name_changed (const QString&))
                );

        connect ( _expression_edit
                , SIGNAL (textChanged ())
                , SLOT (expression_changed ())
                );
      }

      void expression_widget::slot_set_function_name
      ( const QObject* origin
      , const ::xml::parse::type::function_type& function
      , const QString& name
      )
      {
        if (  origin != this
           && &function == &data::proxy::function (proxy())
           )
          {
            set_name (name);
          }
      }

      void expression_widget::slot_set_expression
      ( const QObject* origin
      , const ::xml::parse::type::expression_type& expression
      , const QString& text
      )
      {
        if (  origin != this
           && &expression == &_expression.expression()
           )
          {
            set_expression (text);
          }
      }

      void expression_widget::slot_set_expression_parse_result
      ( const QObject*
      , const ::xml::parse::type::expression_type& expression
      , const QString& text
      )
      {
        if (&expression == &_expression.expression())
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
      expression_widget::set_name (const fhg::util::maybe<std::string>& name)
      {
        set_name (name ? *name : "");
      }
      void expression_widget::set_name (const std::string& name)
      {
        set_name (QString::fromStdString (name));
      }
      void expression_widget::set_name (const QString& name)
      {
        const util::scoped_signal_block block (_name_edit);
        _name_edit->setText (name);
      }
      void expression_widget::set_expression (const std::string& text)
      {
        set_expression (QString::fromStdString (text));
      }
      void expression_widget::set_expression (const QString& text)
      {
         const util::scoped_signal_block block (_expression_edit);
        _expression_edit->setPlainText (text);
      }
      void expression_widget::name_changed (const QString& name_)
      {
        //! \todo implement in base_editor_widget convenience for access to change_manager, function
        data::proxy::root (proxy())->
          change_manager().set_function_name ( this
                                             , data::proxy::function (proxy())
                                             , name_
                                             )
          ;
      }

      void expression_widget::expression_changed ()
      {
        data::proxy::root (proxy())->
          change_manager().set_expression ( this
                                          , _expression.expression()
                                          , _expression_edit->toPlainText()
                                          )
          ;
      }
    }
  }
}

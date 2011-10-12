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

#include <we/expr/parse/parser.hpp>

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
      {
        QGroupBox* group_box (new QGroupBox (tr ("expression")));
        QHBoxLayout* group_box_layout (new QHBoxLayout());
        group_box->setLayout (group_box_layout);

        QSplitter* splitter (new QSplitter ());
        splitter->addWidget (_port_lists);

        QWidget* exp_widget (new QWidget ());
        QVBoxLayout* vbox (new QVBoxLayout ());
        exp_widget->setLayout (vbox);

        QWidget* name_widget (new QWidget ());
        QFormLayout* name_layout (new QFormLayout ());
        name_layout->addRow (tr ("&Name"), _name_edit);
        name_widget->setLayout (name_layout);
        name_layout->setContentsMargins (0, 0, 0, 0);

        vbox->addWidget (name_widget);
        vbox->addWidget (_expression_edit);
        vbox->setContentsMargins (0, 0, 0, 0);

        splitter->addWidget (exp_widget);
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
        {
          const std::string text (_expression_edit->toPlainText().toStdString());

          try
            {
              expr::parse::parser parser (text);

              std::cout << text << std::endl;
            }
          catch (const expr::exception::parse::exception& e)
            {
              std::cout << text << std::endl;

              for (unsigned int k (0); k < e.eaten; ++k)
                {
                  std::cout << " ";
                }
              std::cout << "^" << std::endl;

              std::cout << "parse error: " << e.what() << std::endl;
            }
          catch (const expr::exception::eval::type_error& e)
            {
              std::cout << "type error: " << e.what() << std::endl;
            }
          catch (const expr::exception::eval::divide_by_zero& e)
            {
              std::cout << "divide by zero: " << e.what() << std::endl;
            }
        }

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

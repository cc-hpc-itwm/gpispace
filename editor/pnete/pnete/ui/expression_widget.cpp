// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/ui/expression_widget.hpp>

#include <pnete/data/change_manager.hpp>
#include <pnete/data/handle/function.hpp>
#include <pnete/ui/port_lists_widget.hpp>

#include <xml/parse/parser.hpp>
#include <xml/parse/type/function.hpp>

#include <util/qt/scoped_signal_block.hpp>

#include <we/mgmt/context.hpp>
#include <we/mgmt/type/activity.hpp>
#include <we/type/value/read.hpp>
#include <we/util/token.hpp>

#include <QAction>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
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

        QGroupBox* group_box (new QGroupBox (tr ("expression")));
        QHBoxLayout* group_box_layout (new QHBoxLayout());
        group_box->setLayout (group_box_layout);
        group_box_layout->addWidget (splitter);

        QHBoxLayout* hbox (new QHBoxLayout ());
        hbox->addWidget (group_box);
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
        set_expression (expression.get().expression("\n"));

        QAction* run_pnetc_action (new QAction (tr ("run_pnetc"), this));
        connect (run_pnetc_action, SIGNAL (triggered()), SLOT (run_pnetc()));
        addAction (run_pnetc_action);
      }

      void expression_widget::slot_set_function_name
        ( const QObject* origin
        , const data::handle::function& fun
        , const QString& name
        )
      {
        if (origin != this && is_my_function (fun))
          {
            set_name (name);
          }
      }

      void expression_widget::slot_set_expression
        ( const QObject* origin
        , const data::handle::expression& expression
        , const QString& text
        )
      {
        if (origin != this && is_my_expression (expression))
          {
            set_expression (text);
          }
      }

      void expression_widget::slot_set_expression_parse_result
        ( const QObject*
        , const data::handle::expression& expression
        , const QString& text
        )
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
        _function.set_name (this, name);
      }

      void expression_widget::expression_changed ()
      {
        _expression.set_content (this, _expression_edit->toPlainText());
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

      //! \note Context copied from we-eval.
      class eval_context : public we::mgmt::context
      {
      public:
        virtual int handle_internally (we::mgmt::type::activity_t& act, net_t&)
        {
          act.inject_input();

          while (act.can_fire())
          {
            we::mgmt::type::activity_t sub (act.extract());
            sub.inject_input();
            sub.execute (this);
            act.inject (sub);
          }

          act.collect_output ();

          return 0;
        }

        virtual int handle_internally (we::mgmt::type::activity_t& act, mod_t& mod)
        {
          // module::call (loader, act, mod);

          return 0;
        }

        virtual int handle_internally (we::mgmt::type::activity_t& , expr_t&)
        {
          return 0;
        }

        virtual int handle_externally (we::mgmt::type::activity_t& act, net_t& n)
        {
          return handle_internally (act, n);
        }

        virtual int handle_externally (we::mgmt::type::activity_t& act, mod_t& mod)
        {
          return handle_internally (act, mod);
        }

        virtual int handle_externally (we::mgmt::type::activity_t& act, expr_t& e)
        {
          return handle_internally (act, e);
        }
      };

      struct output_port_and_token : std::iterator< std::output_iterator_tag
                                                  , output_port_and_token
                                                  >
      {
        output_port_and_token const & operator *() const { return *this; }
        output_port_and_token const & operator++() const { return *this; }
        output_port_and_token const & operator=
          (const we::mgmt::type::activity_t::token_on_port_t & subject) const
        {
          std::stringstream tmp;
          tmp << "on " << subject.second << ": " << subject.first;
          QMessageBox msgBox;
          msgBox.setText (QString::fromStdString (tmp.str()));
          msgBox.exec();
          return *this;
        }
      };

      void expression_widget::run_pnetc()
      try
      {
        xml::parse::state::type state;

        const xml::parse::id::ref::function function (_function.get().clone());
        xml::parse::post_processing_passes (function, &state);

        we::mgmt::type::activity_t activity
          (xml::parse::xml_to_we (function, state));

        BOOST_FOREACH ( const std::string& port_name
                      , activity.transition().port_names (we::type::PORT_IN)
                      )
        {
          bool retry (true);
          while (retry)
          {
            bool ok;
            const std::string value
              ( QInputDialog::getText ( this
                                      , tr ("value_for_input_token")
                                      , tr ("enter_value_for_input_port_%1")
                                      .arg (QString::fromStdString (port_name))
                                      , QLineEdit::Normal
                                      , "[]"
                                      , &ok
                                      ).toStdString()
              );
            if (!ok)
            {
              return;
            }

            std::size_t k (0);
            std::string::const_iterator begin (value.begin());
            fhg::util::parse::position pos (k, begin, value.end());

            try
            {
              try
              {
                we::util::token::put (activity, port_name, ::value::read (pos));
                retry = false;
              }
              catch (const expr::exception::parse::exception& e)
              {
                //! \todo fixed width font
                std::stringstream temp;
                temp << e.what() << std::endl;
                temp << value << std::endl;
                temp << std::string (e.eaten, ' ') << "^" << std::endl;
                throw std::runtime_error (temp.str().c_str());
              }
            }
            catch (const std::runtime_error& e)
            {
              QMessageBox msgBox;
              msgBox.setText (e.what());
              msgBox.setIcon (QMessageBox::Critical);
              msgBox.exec();
              retry = true;
            }
          }
        }

        activity.inject_input();

        eval_context context;
        activity.execute (&context);
        activity.collect_output();

        std::copy ( activity.output().begin()
                  , activity.output().end()
                  , output_port_and_token()
                  );
      }
      catch (const std::runtime_error& e)
      {
        QMessageBox msgBox;
        msgBox.setText (e.what());
        msgBox.setIcon (QMessageBox::Critical);
        msgBox.exec();
      }
    }
  }
}

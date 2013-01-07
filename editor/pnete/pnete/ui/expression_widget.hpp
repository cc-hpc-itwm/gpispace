// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef _PNETE_UI_EXPRESSION_WIDGET_HPP
#define _PNETE_UI_EXPRESSION_WIDGET_HPP 1

#include <QObject>

#include <pnete/data/proxy.hpp>

#include <pnete/data/handle/expression.hpp>

#include <pnete/ui/base_editor_widget.hpp>

#include <boost/optional.hpp>

class QWidget;
class QTextEdit;
class QLineEdit;

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      class port_lists_widget;

      class expression_widget : public base_editor_widget
      {
        Q_OBJECT;

      public:
        expression_widget ( data::proxy::type&
                          , const data::handle::expression&
                          , const data::handle::function&
                          , const QStringList& types
                          , QWidget* parent = NULL
                          );
        void set_expression (const QString&);
        void set_expression (const std::string&);

        void set_name (const boost::optional<std::string>&);
        void set_name (const QString&);
        void set_name (const std::string&);

        void set_expression_parse_result (const QString&);

      private slots:
        void name_changed (const QString& name_);
        void expression_changed ();

        void slot_set_function_name ( const QObject*
                                    , const data::handle::function&
                                    , const QString&
                                    );
        void slot_set_expression ( const QObject*
                                 , const data::handle::expression&
                                 , const QString&
                                 );
        void slot_set_expression_parse_result
             ( const QObject*
             , const data::handle::expression&
             , const QString&
             );

      private:
        bool is_my_function (const data::handle::function&);
        bool is_my_expression (const data::handle::expression&);

        data::handle::expression _expression;
        data::handle::function _function;
        port_lists_widget* _port_lists;
        QTextEdit* _expression_edit;
        QLineEdit* _name_edit;
        QTextEdit* _parse_result;
      };
    }
  }
}

#endif

// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef _PNETE_UI_DOCUMENT_VIEW_HPP
#define _PNETE_UI_DOCUMENT_VIEW_HPP 1

#include <QObject>

#include <pnete/data/proxy.hpp>

#include <pnete/ui/dock_widget.hpp>

#include <fhg/util/maybe.hpp>

class QString;

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      class base_editor_widget;
      namespace graph
      {
        class scene;
      }

      class document_view : public dock_widget
      {
        Q_OBJECT;

      public:
        document_view (const data::proxy::type& proxy);

        base_editor_widget* widget() const;
        void setWidget (base_editor_widget* widget);

      protected:
        void set_title (const fhg::util::maybe<std::string>&);

      signals:
        void focus_gained (QWidget*);

      public slots:
        void slot_set_function_name ( ::xml::parse::type::function_type&
                                    , const QString&
                                    );
      private slots:
        void visibility_changed (bool);

      private:
        virtual QString fallback_title() const = 0;
      };

      class expression_view : public document_view
      {
        Q_OBJECT;

      public:
        expression_view ( data::proxy::type&
                        , data::proxy::expression_proxy::data_type&
                        );
      private:
        virtual QString fallback_title() const;
      };

      class mod_view : public document_view
      {
        Q_OBJECT;

      public:
        mod_view ( data::proxy::type&
                 , data::proxy::mod_proxy::data_type&
                 );

      private:
        virtual QString fallback_title() const;
      };

      class net_view : public document_view
      {
        Q_OBJECT;

      public:
        net_view ( data::proxy::type&
                 , data::proxy::net_proxy::data_type&
                 , graph::scene*
                 );
      private:
        virtual QString fallback_title() const;
      };
    }
  }
}

#endif

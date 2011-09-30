// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef _PNETE_UI_VIEW_MANAGER_HPP
#define _PNETE_UI_VIEW_MANAGER_HPP 1

#include <QObject>

#include <pnete/data/proxy.hpp>

class QAction;
class QWidget;

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      class document_widget;
      class editor_window;

      class view_manager : public QObject
      {
        Q_OBJECT

        public:
          view_manager (editor_window* parent);

        public slots:
          void focus_changed (QWidget*);

        // net_view
        //! \todo QActions!, disable for non-nets
          void current_view_zoom (int);
          void current_view_zoom_in();
          void current_view_zoom_out();
          void current_view_reset_zoom();

          void current_scene_add_transition();
          void current_scene_add_place();
          void current_scene_add_struct();
          void current_scene_auto_layout();

        // document, general
          void duplicate_active_widget();
          void create_widget (data::proxy::type &);
          void current_widget_close();
        void save_file();

        signals:
        // net_view
          void zoomed (int);
        //void data_changed (function_type*);

      public:
        QAction* save_current_file_action();

        private:
          editor_window* _editor_window;

          QList<document_widget*> _accessed_widgets;

          void add_on_top_of_current_widget (document_widget* w);

        QAction* _save_current_file;
        void initialize_actions();

        void disable_file_actions();
        void enable_file_actions();
      };
    }
  }
}

#endif

// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef _PNETE_UI_VIEW_MANAGER_HPP
#define _PNETE_UI_VIEW_MANAGER_HPP 1

#include <QObject>
#include <QUndoGroup>

#include <pnete/data/proxy.hpp>

#include <QStack>

class QAction;
class QWidget;
class QUndoView;

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      class document_view;
      class editor_window;

      class view_manager : public QObject
      {
        Q_OBJECT;

      public:
        view_manager (editor_window* parent);

        QAction* action_save_current_file();

      public slots:
        void focus_changed (QWidget*, QWidget*);
        // document, general
        void duplicate_active_widget();
        void create_widget (data::proxy::type &);
        void current_widget_close();
        void save_file();

        QUndoView* create_undo_view (QWidget* parent = NULL) const;
        QUndoGroup* undo_group() const;

      private:
        editor_window* _editor_window;

        QStack<document_view*> _accessed_widgets;

        void add_on_top_of_current_widget (document_view* w);

        QAction* _action_save_current_file;

        QUndoGroup* _undo_group;
      };
    }
  }
}

#endif

// bernd.loerwald@itwm.fraunhofer.de

#include "view_manager.hpp"

#include <stdexcept>

#include <QString>
#include <QWidget>

#include "editor_window.hpp"
#include "dockable_graph_view.hpp"
#include "GraphScene.hpp"
#include "GraphView.hpp"
#include "util.hpp"

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      view_manager::view_manager (editor_window* parent)
      : QObject (NULL)
      , _editor_window (parent)
      , _open_files()
      , _accessed_widgets()
      , _current_view (NULL)
      , _current_scene (NULL)
      {
      }

      void view_manager::focus_changed (QWidget* widget)
      {
        dockable_graph_view* current_widget
              (util::first_parent_being_a<dockable_graph_view> (widget));

        if (!current_widget)
        {
          throw std::runtime_error ("a view not being on a d_g_v got focused.");
        }

        _accessed_widgets.append (current_widget);

        current_widget->setFocus();
        current_widget->raise();

        GraphView* old_view (_current_view);
        _current_view = current_widget->graph_view();
        if (old_view != _current_view)
        {
          emit view_changed (_current_view);
        }

        graph::Scene* old_scene (_current_scene);
        _current_scene = _current_view->scene();

        if (!_current_scene)
        {
          throw std::runtime_error ("there is a view without a scene.");
        }

        if (old_scene != _current_scene)
        {
          emit scene_changed (_current_scene);
        }
      }

      // -- current widget --

      void view_manager::add_on_top_of_current_widget (dockable_graph_view* w)
      {
        if (!_accessed_widgets.isEmpty())
        {
          _editor_window->tabifyDockWidget (_accessed_widgets.back(), w);
        }
        else
        {
          _editor_window->
              addDockWidget (Qt::LeftDockWidgetArea, w, Qt::Horizontal);
        }
        connect ( w->graph_view()
                , SIGNAL (focus_gained (QWidget*))
                , SLOT (focus_changed (QWidget*))
                );
        connect ( w->graph_view()
                , SIGNAL (zoomed (int))
                , SIGNAL (zoomed (int))
                );

        //! \todo For some reason, my window manager does not raise the tab.
        w->graph_view()->setFocus();
        focus_changed (w->graph_view());
      }

      void view_manager::create_new_view_for_current_scene()
      {
        add_on_top_of_current_widget (new dockable_graph_view (_current_scene));
      }
      void view_manager::create_new_scene_and_view()
      {
        add_on_top_of_current_widget
            (new dockable_graph_view (new graph::Scene (this)));
      }
      void view_manager::current_document_close()
      {
        if (_accessed_widgets.isEmpty())
        {
          return;
        }
        dockable_graph_view* current (_accessed_widgets.back());
        _accessed_widgets.removeAll (current);
        _editor_window->removeDockWidget (current);
        delete current;
        if (!_accessed_widgets.isEmpty())
        {
          _accessed_widgets.back()->graph_view()->setFocus();
        }
      }
      void view_manager::create_view_for_file (const QString& filename)
      {
        graph::Scene* scene (NULL);
        if (!_open_files.contains (filename))
        {
          _open_files[filename] = new graph::Scene (filename);
        }

        add_on_top_of_current_widget
            (new dockable_graph_view (_open_files[filename]));
      }
      void view_manager::save_current_scene (const QString& filename)
      {
        if (_open_files.contains (filename))
        {
          throw std::runtime_error
              ("there already is a file open with this name.");
        }
        _open_files[filename] = _current_scene;
        _open_files[filename]->save (filename);
        _accessed_widgets.back()->setWindowTitle
            (QString (_open_files[filename]->name()).append("[*]"));
      }

      // -- current scene --

      void view_manager::current_scene_add_transition()
      {
        _current_scene->slot_add_transition();
      }
      void view_manager::current_scene_add_place()
      {
        _current_scene->slot_add_place();
      }
      void view_manager::current_scene_add_struct()
      {
        _current_scene->slot_add_struct();
      }
      void view_manager::current_scene_auto_layout()
      {
        _current_scene->auto_layout();
      }

      // -- current view --

      void view_manager::current_view_zoom (int level)
      {
        _current_view->zoom (level);
      }
      void view_manager::current_view_zoom_in()
      {
        _current_view->zoom_in();
      }
      void view_manager::current_view_zoom_out()
      {
        _current_view->zoom_out();
      }
      void view_manager::current_view_reset_zoom()
      {
        _current_view->reset_zoom();
      }
    }
  }
}

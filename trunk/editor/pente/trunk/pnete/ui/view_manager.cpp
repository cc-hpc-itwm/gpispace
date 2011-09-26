// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/ui/view_manager.hpp>

#include <stdexcept>
#include <iostream>

#include <QString>
#include <QWidget>

#include <pnete/ui/editor_window.hpp>
#include <pnete/ui/document_widget.hpp>
#include <pnete/ui/base_editor_widget.hpp>
#include <pnete/ui/GraphView.hpp>

#include <pnete/util.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      view_manager::view_manager (editor_window* parent)
        : QObject (NULL)
        , _editor_window (parent)
        , _accessed_widgets()
      {}

      void view_manager::focus_changed (QWidget* widget)
      {
        document_widget* current_widget
              (util::first_parent_being_a<document_widget> (widget));

        if (!current_widget)
        {
          throw std::runtime_error ( "focus changed on a widget not "
                                     "on a document_widget."
                                   );
        }

        _accessed_widgets.append (current_widget);

        //! \todo enable and disable actions

//         GraphView* old_view (_current_view);
//         _current_view = current_widget->graph_view();
//         if (old_view != _current_view)
//         {
//           emit view_changed (_current_view);
//         }

//         graph::scene* old_scene (_current_scene);
//         _current_scene = _current_view->scene();

//         if (!_current_scene)
//         {
//           throw std::runtime_error ("there is a view without a scene.");
//         }

//         if (old_scene != _current_scene)
//         {
//           emit scene_changed (_current_scene);
//         }
      }

      // -- document, general --

      void view_manager::add_on_top_of_current_widget (document_widget* w)
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
        connect ( w->widget()
                , SIGNAL (focus_gained (QWidget*))
                , SLOT (focus_changed (QWidget*))
                );
//         connect ( w->graph()
//                 , SIGNAL (zoomed (int))
//                 , SIGNAL (zoomed (int))
//                 );

        w->show();
        w->raise();
        focus_changed (w->widget());
      }

      void view_manager::duplicate_active_widget()
      {
        if (_accessed_widgets.isEmpty())
        {
          return;
        }

        create_widget (_accessed_widgets.back()->widget()->proxy());
      }
      void view_manager::current_widget_close()
      {
        if (_accessed_widgets.isEmpty())
        {
          return;
        }
        document_widget* current (_accessed_widgets.back());
        _accessed_widgets.removeAll (current);
        _editor_window->removeDockWidget (current);
        delete current;
        if (!_accessed_widgets.isEmpty())
        {
          _accessed_widgets.back()->widget()->setFocus();
        }
      }
      void view_manager::create_widget (data::proxy::type& proxy)
      {
        add_on_top_of_current_widget
          (data::proxy::document_widget_factory (proxy));
      }

      // net_view, ACTIONS!

      // -- current scene --

      void view_manager::current_scene_add_transition()
      {
        //_current_scene->slot_add_transition();
      }
      void view_manager::current_scene_add_place()
      {
        //_current_scene->slot_add_place();
      }
      void view_manager::current_scene_add_struct()
      {
        //_current_scene->slot_add_struct();
      }
      void view_manager::current_scene_auto_layout()
      {
        //_current_scene->auto_layout();
      }

      // -- current view --

      void view_manager::current_view_zoom (int level)
      {
        //_current_view->zoom (level);
      }
      void view_manager::current_view_zoom_in()
      {
        //_current_view->zoom_in();
      }
      void view_manager::current_view_zoom_out()
      {
        //_current_view->zoom_out();
      }
      void view_manager::current_view_reset_zoom()
      {
        //_current_view->reset_zoom();
      }
    }
  }
}

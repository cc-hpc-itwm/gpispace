// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/ui/view_manager.hpp>

#include <stdexcept>
#include <iostream>

#include <QAction>
#include <QFileDialog>
#include <QDir>
#include <QString>
#include <QWidget>
#include <QUndoView>

#include <pnete/ui/editor_window.hpp>
#include <pnete/ui/document_view.hpp>
#include <pnete/ui/base_editor_widget.hpp>
#include <pnete/ui/GraphView.hpp>

#include <pnete/data/manager.hpp>

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
        , _action_save_current_file (new QAction (tr ("save"), this))
        , _undo_group (new QUndoGroup (this))
      {
        _action_save_current_file->setShortcuts (QKeySequence::Save);
        _action_save_current_file->setEnabled (false);
        connect ( _action_save_current_file
                , SIGNAL (triggered())
                , SLOT (save_file())
                );
      }

      void view_manager::save_file()
      {
        if (_accessed_widgets.empty())
        {
          return;
        }

        QString filename ( QFileDialog::getSaveFileName
                           ( _editor_window
                           , tr ("Save")
                           , QDir::homePath()
                           , tr ("XML files (*.xml);; All files (*)")
                           )
                         );

        if (filename.isEmpty())
        {
          return;
        }

        if (!filename.endsWith (".xml"))
        {
          filename.append (".xml");
        }

        data::manager::instance().save
          ( data::proxy::root (_accessed_widgets.top()->widget()->proxy())
          , filename
          );
      }

      void view_manager::focus_changed (QWidget* widget)
      {
        document_view* current_view
              (util::first_parent_being_a<document_view> (widget));

        if (!current_view)
        {
          throw std::runtime_error ( "focus changed on a widget not "
                                     "on a document_widget."
                                   );
        }

        _accessed_widgets.push (current_view);

        data::proxy::root (current_view->widget()->proxy())->change_manager()
          .setActive(true);

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

      void view_manager::add_on_top_of_current_widget (document_view* w)
      {
        if (!_accessed_widgets.empty())
          {
            _editor_window->tabifyDockWidget (_accessed_widgets.top(), w);
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
        connect ( w
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
        if (!_accessed_widgets.empty())
          {
            create_widget (_accessed_widgets.top()->widget()->proxy());
          }
      }
      void view_manager::current_widget_close()
      {
        if (!_accessed_widgets.empty())
        {
          document_view* current (_accessed_widgets.top());
          _accessed_widgets.pop ();
          _editor_window->removeDockWidget (current);
          delete current;

          if (!_accessed_widgets.empty())
            {
              _accessed_widgets.top()->widget()->setFocus();
            }
          else
            {
              _action_save_current_file->setEnabled (false);
            }
        }
      }
      void view_manager::create_widget (data::proxy::type& proxy)
      {
        _undo_group->addStack (&data::proxy::root (proxy)->change_manager());

        add_on_top_of_current_widget
          (data::proxy::document_view_factory (proxy));

        _action_save_current_file->setEnabled (true);
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

      QAction* view_manager::action_save_current_file()
      {
        return _action_save_current_file;
      }

      QUndoView* view_manager::create_undo_view (QWidget* parent) const
      {
        return new QUndoView (undo_group(), parent);
      }

      QUndoGroup* view_manager::undo_group() const
      {
        return _undo_group;
      }
    }
  }
}

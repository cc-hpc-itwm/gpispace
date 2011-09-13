// bernd.loerwald@itwm.fraunhofer.de

#include "view_manager.hpp"

#include <stdexcept>

#include <QString>
#include <QWidget>

#include "convenience_splitter.hpp"
#include "GraphScene.hpp"
#include "GraphView.hpp"
#include "splittable_tab_widget.hpp"
#include "util.hpp"

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      splittable_tab_widget* view_manager::create_default_tab_widget()
      {
        splittable_tab_widget* new_widget (new splittable_tab_widget (this));
        new_widget->add_tab_for_scene (new graph::Scene (this));
        return new_widget;
      }

      view_manager::view_manager (QWidget* parent)
      : _open_files()
      //! \note splitter takes 'this' two times. I'd rather not put it here.
      , _splitter (NULL)
      , _current_tab_widget (NULL)
      , _current_view (NULL)
      , _current_scene (NULL)
      {
        _splitter = new convenience_splitter (this, parent);
        splittable_tab_widget* default_tab (create_default_tab_widget());
        _splitter->add (default_tab);
        //! \note We fake the signal here, as it would not be called during construction.
        focus_changed (default_tab);
      }

      void view_manager::focus_changed (QWidget* widget)
      {
        _current_tab_widget
            = util::first_parent_being_a<splittable_tab_widget>(widget);

        if (!_current_tab_widget)
        {
          throw std::runtime_error ("a view not being on a tab got focused.");
        }

        _current_tab_widget->setFocus();

        GraphView* old_view (_current_view);
        _current_view = qobject_cast<GraphView*>
            (_current_tab_widget->currentWidget());

        if (!_current_view)
        {
          throw std::runtime_error ("there is a non-view on the selected tab.");
        }

        if (old_view != _current_view)
        {
          emit view_changed (_current_view);
        }

        graph::Scene* old_scene (_current_scene);
        _current_scene = qobject_cast<graph::Scene*> (_current_view->scene());

        if (!_current_scene)
        {
          throw std::runtime_error ("there is a view without a scene.");
        }

        if (old_scene != _current_scene)
        {
          emit scene_changed (_current_scene);
        }
      }

      graph::Scene* view_manager::current_scene() const
      {
        return _current_scene;
      }
      GraphView* view_manager::current_view() const
      {
        return _current_view;
      }
      convenience_splitter* view_manager::splitter() const
      {
        return _splitter;
      }
      splittable_tab_widget* view_manager::current_tab_widget() const
      {
        return _current_tab_widget;
      }

      void view_manager::create_new_view_for_current_scene()
      {
        _current_tab_widget->add_tab_for_scene (current_scene());
      }

      void view_manager::create_new_scene_and_view()
      {
        _current_tab_widget->add_tab_for_scene (new graph::Scene (this));
      }

      void view_manager::current_scene_add_transition()
      {
        current_scene()->slot_add_transition();
      }
      void view_manager::current_scene_add_place()
      {
        current_scene()->slot_add_place();
      }
      void view_manager::current_scene_add_struct()
      {
        current_scene()->slot_add_struct();
      }

      void view_manager::zoom_current_view (int level)
      {
        current_view()->zoom (level);
      }

      void view_manager::create_view_for_file (const QString& filename)
      {
        graph::Scene* scene (NULL);
        if (!_open_files.contains (filename))
        {
          _open_files[filename] = new graph::Scene (filename);
        }

        _current_tab_widget->add_tab_for_scene (_open_files[filename]);
      }

      void view_manager::save_current_scene (const QString& filename)
      {
        if (_open_files.contains (filename))
        {
          throw std::runtime_error
              ("there already is a file open with this name.");
        }
        _open_files[filename] = current_scene();
        _open_files[filename]->save (filename);
        _current_tab_widget->setTabText( _current_tab_widget->currentIndex()
                                       , _open_files[filename]->name()
                                       );
      }
    }
  }
}

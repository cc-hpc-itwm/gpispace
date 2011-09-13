// bernd.loerwald@itwm.fraunhofer.de

#include "splittable_tab_widget.hpp"

#include <QMenu>
#include <QToolButton>
#include <QWidget>

#include "GraphScene.hpp"
#include "GraphView.hpp"
#include "view_manager.hpp"

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      splittable_tab_widget::splittable_tab_widget ( view_manager* manager
                                                   , QWidget* parent
                                                   )
      : QTabWidget (parent)
      , _view_manager (manager)
      , _menu (new QMenu (this))
      {
        QAction* split_h (_menu->addAction (tr ("horizontal_split")));
        QAction* split_v (_menu->addAction (tr ("vertical_split")));
        QAction* remove (_menu->addAction (tr ("remove_this")));

        connect (split_h, SIGNAL (triggered()), SLOT (split_horizontal()));
        connect (split_v, SIGNAL (triggered()), SLOT (split_vertical()));
        connect (remove, SIGNAL (triggered()), SLOT (remove_this()));

        //! \todo Icon.
        QToolButton* corner_button (new QToolButton (this));
        corner_button->setMenu (_menu);
        corner_button->setPopupMode (QToolButton::InstantPopup);
        setCornerWidget (corner_button);

        setTabsClosable (true);
        setDocumentMode (true);
        setMovable (true);

        connect ( this
                , SIGNAL (tabCloseRequested (int))
                , SLOT (tab_close_requested (int))
                );
      }

      //! \note These are only re-emitting them with "this" added.
      void splittable_tab_widget::split_vertical()
      {
        emit split (this, Qt::Vertical);
      }
      void splittable_tab_widget::split_horizontal()
      {
        emit split (this, Qt::Horizontal);
      }
      void splittable_tab_widget::remove_this()
      {
        emit remove (this);
      }

      void splittable_tab_widget::tab_close_requested (int index)
      {
        //! \todo Detect, if this is the last open view for this document and alert / save.
        QWidget* closed_view (widget (index));
        removeTab (index);
        delete closed_view;

        if (count() == 0)
        {
          emit remove (this);
        }
      }

      void splittable_tab_widget::close_current_tab()
      {
        tab_close_requested (currentIndex());
      }

      void
      splittable_tab_widget::add_tab_for_scene (graph::Scene* scene)
      {
        GraphView* view (new GraphView (scene, NULL));
        setCurrentIndex (addTab (view, scene->name()));

        _view_manager->connect ( view
                               , SIGNAL (focus_gained (QWidget*))
                               , SLOT (focus_changed (QWidget*))
                               );
        _view_manager->connect ( view
                               , SIGNAL (zoomed (int))
                               , SIGNAL (zoomed (int))
                               );

        view->setFocus();
      }
    }
  }
}

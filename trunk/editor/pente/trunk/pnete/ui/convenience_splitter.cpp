// bernd.loerwald@itwm.fraunhofer.de

#include "convenience_splitter.hpp"

#include <QList>
#include <QWidget>

#include "view_manager.hpp"
#include "splittable_tab_widget.hpp"

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {

      convenience_splitter::convenience_splitter ( view_manager* view_manager_
                                                 , const Qt::Orientation& orient
                                                 , QWidget* parent
                                                 )
      : QSplitter (orient, parent)
      , _view_manager (view_manager_)
      {
      }

      convenience_splitter::convenience_splitter ( view_manager* view_manager_
                                                 , QWidget* parent
                                                 )
      : QSplitter (parent)
      , _view_manager (view_manager_)
      {
      }

      void convenience_splitter::add (splittable_tab_widget* widget)
      {
        addWidget (widget);
        connect ( widget
                , SIGNAL (split (splittable_tab_widget*, const Qt::Orientation&))
                , SLOT (split (splittable_tab_widget*, const Qt::Orientation&))
                );
        connect ( widget
                , SIGNAL (remove (splittable_tab_widget*))
                , SLOT (remove (splittable_tab_widget*))
                );
      }

      void convenience_splitter::split ( splittable_tab_widget* to_split
                                       , const Qt::Orientation& want_orientation
                                       )
      {
        if (count() == 1)
        {
          setOrientation (want_orientation);
        }

        splittable_tab_widget* new_widget
            (_view_manager->create_default_tab_widget());

        if (orientation() == want_orientation)
        {
          add (new_widget);
        }
        else
        {
          const int index (indexOf (to_split));
          to_split->setParent (NULL);

          convenience_splitter* new_splitter
              ( new convenience_splitter ( _view_manager
                                         , want_orientation
                                         , this
                                         )
              );
          new_splitter->add (to_split);
          new_splitter->add (new_widget);

          insertWidget (index, new_splitter);
        }
      }

      void convenience_splitter::remove (splittable_tab_widget* widget)
      {
        delete widget;
        //! \todo Now that we have removed this, we might simplify the tree.
        // for example, when we now have no childs anymore, we can remove this.

        //! \note If we have just removed our last child, insert a default tab.
        const QList<splittable_tab_widget*> list
            (_view_manager->splitter()->findChildren<splittable_tab_widget*>());
        if (list.count() == 0)
        {
          add (_view_manager->create_default_tab_widget());
        }
        else
        {
          //! \note Fix focus pointers in view_manager.
          list.at (0)->currentWidget()->setFocus();
        }
      }
    }
  }
}

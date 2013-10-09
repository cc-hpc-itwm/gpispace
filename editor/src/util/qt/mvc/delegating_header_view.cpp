// bernd.loerwald@itwm.fraunhofer.de

#include <util/qt/mvc/delegating_header_view.hpp>

#include <util/qt/boost_connect.hpp>
#include <util/qt/mini_button.hpp>
#include <util/qt/mvc/header_delegate.hpp>
#include <util/qt/painter_state_saver.hpp>

#include <boost/bind.hpp>

#include <QContextMenuEvent>
#include <QKeyEvent>
#include <QMenu>
#include <QPainter>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      namespace mvc
      {
        delegating_header_view::delegating_header_view (QWidget* parent)
          : QHeaderView (Qt::Horizontal, parent)
          , _delegate (NULL)
        {
          connect ( this
                  , SIGNAL (sectionResized (int, int, int))
                  , SLOT (set_editor_geometry())
                  );
          connect ( this
                  , SIGNAL (sectionMoved (int, int, int))
                  , SLOT (set_editor_geometry())
                  );
          connect ( this
                  , SIGNAL (sectionDoubleClicked (int))
                  , SLOT (request_editor (int))
                  );

          setSizePolicy (sizePolicy().horizontalPolicy(), QSizePolicy::Preferred);
          setFocusPolicy (Qt::ClickFocus);
        }

        void delegating_header_view::setModel (QAbstractItemModel* m)
        {
          if (model())
          {
            model()->disconnect (this);
          }

          QHeaderView::setModel (m);

          if (model())
          {
            connect ( model()
                    , orientation() == Qt::Horizontal
                    ? SIGNAL (columnsInserted (const QModelIndex&, int, int))
                    : SIGNAL (rowsInserted (const QModelIndex&, int, int))
                    , SLOT (sections_inserted (const QModelIndex&, int, int))
                    );
            connect ( model()
                    , orientation() == Qt::Horizontal
                    ? SIGNAL (columnsRemoved (const QModelIndex&, int, int))
                    : SIGNAL (rowsRemoved (const QModelIndex&, int, int))
                    , SLOT (sections_removed (const QModelIndex&, int, int))
                    );
            connect ( model()
                    , SIGNAL (headerDataChanged (Qt::Orientation, int, int))
                    , SLOT (data_changed (Qt::Orientation, int, int))
                    );

            _delegates.insert (0, model()->columnCount(), NULL);
          }
        }

        void delegating_header_view::delegate_for_section
          (int section, header_delegate* delegate)
        {
          if (section < count())
          {
            _delegates[section] = delegate;
          }
        }

        header_delegate* delegating_header_view::delegate_for_section
          (int section) const
        {
          return section < count() && _delegates[section]
            ? _delegates[section] : _delegate;
        }

        void delegating_header_view::delegate (header_delegate* delegate)
        {
          _delegate = delegate;
          update();
        }

        void delegating_header_view::paintSection ( QPainter* painter
                                                  , const QRect& rect
                                                  , int logical_index
                                                  ) const
        {
          const_cast<delegating_header_view*> (this)->set_editor_geometry();

          {
            const painter_state_saver state_saver (painter);
            QHeaderView::paintSection (painter, rect, logical_index);
          }

          if ( delegate_for_section (logical_index)
             && (!_editor.section || *_editor.section != logical_index)
             )
          {
            const painter_state_saver state_saver (painter);
            painter->setClipRect (rect);
            delegate_for_section (logical_index)->paint
              (painter, rect, section_index (this, logical_index));
          }
        }

        QSize delegating_header_view::sizeHint() const
        {
          QSize s (QHeaderView::sizeHint());
          if (_editor.widget)
          {
            s.setHeight (qMax (s.height(), _editor.widget->height()));
          }
          return s;
        }

        void delegating_header_view::keyPressEvent (QKeyEvent* event)
        {
          if (event->key() == Qt::Key_Escape)
          {
            close_editor();
          }
          QHeaderView::keyPressEvent (event);
        }

        void delegating_header_view::contextMenuEvent (QContextMenuEvent* event)
        {
          const int section (logicalIndexAt (event->x()));
          if (section != -1 && delegate_for_section (section))
          {
            QMenu* menu ( delegate_for_section (section)->menu_for_section
                          (section_index (this, section))
                        );
            if (menu)
            {
              menu->exec (event->globalPos());
              delete menu;
              return;
            }
          }

          QHeaderView::contextMenuEvent (event);
        }

        void delegating_header_view::sections_inserted
          (const QModelIndex&, int from, int to)
        {
          _delegates.insert (from, to - from + 1, NULL);
          update();
        }

        void delegating_header_view::sections_removed
          (const QModelIndex&, int from, int to)
        {
          _delegates.remove (from, to - from + 1);
          if (_editor.section && *_editor.section >= from && *_editor.section <= to)
          {
            close_editor();
          }
          update();
        }

        void delegating_header_view::request_editor (int section)
        {
          close_editor();

          if ( delegate_for_section (section)
            && delegate_for_section (section)->can_edit_section
                 (section_index (this, section))
             )
          {
            _editor.section = section;

            _editor.close_button
              = new mini_button (QStyle::SP_TitleBarCloseButton, this);
            _editor.close_button->show();
            connect (_editor.close_button, SIGNAL (clicked()), SLOT (close_editor()));

            const QRect geom (editor_geometry());

            _editor.widget = delegate_for_section (section)->create_editor
              (geom, this, section_index (this, section));
            _editor.widget->show();

            invalidate_cached_size_hint();
          }
        }

        void delegating_header_view::data_changed
          (Qt::Orientation o, int first, int last)
        {
          if ( o == orientation() && _editor.section
             && *_editor.section >= first && *_editor.section <= last
             )
          {
            delegate_for_section (*_editor.section)->update_editor
              (section_index (this, *_editor.section), _editor.widget);
          }
        }

        namespace
        {
          int margin (const QWidget* wid)
          {
            return
              wid->style()->pixelMetric (QStyle::PM_HeaderGripMargin, 0, wid) + 1;
          }
        }

        void delegating_header_view::set_editor_geometry()
        {
          if (_editor.section && _editor.widget && _editor.close_button)
          {
            _editor.widget->setGeometry (editor_geometry());
            QRect close_button_geom (_editor.close_button->geometry());
            const int inset (margin (this));
            close_button_geom.moveTopLeft
              (editor_geometry().topRight() + QPoint (inset, inset));
            _editor.close_button->setGeometry (close_button_geom);
          }
        }

        void delegating_header_view::close_editor()
        {
          if (_editor.section)
          {
            delegate_for_section (*_editor.section)->release_editor
              (section_index (this, *_editor.section), _editor.widget);
            _editor.widget = NULL;

            delete _editor.close_button;
            _editor.close_button = NULL;

            _editor.section = boost::none;

            invalidate_cached_size_hint();
          }
        }

        QRect delegating_header_view::editor_geometry() const
        {
          const int inset (margin (this));

          return QRect ( sectionViewportPosition (*_editor.section) + inset
                       , 0
                       , sectionSize (*_editor.section) - inset * 3
                       - _editor.close_button->width()
                       , _editor.widget ? _editor.widget->height() : height()
                       );
        }

        void delegating_header_view::invalidate_cached_size_hint()
        {
          //! \note We want to call invalidateCachedSizeHint(), but.. can't. Hack:
          // case QEvent::StyleChange:
          //     d->invalidateCachedSizeHint();
          //     resizeSections();
          //     emit geometriesChanged();
          //     break;
          QEvent event (QEvent::StyleChange);
          QHeaderView::viewportEvent (&event);
        }
      }
    }
  }
}

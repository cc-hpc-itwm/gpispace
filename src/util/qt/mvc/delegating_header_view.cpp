// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <util/qt/mvc/delegating_header_view.hpp>

#include <util-qt/widget/mini_button.hpp>
#include <util/qt/mvc/header_delegate.hpp>
#include <util-qt/painter_state_saver.hpp>

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
          , _delegate (nullptr)
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
          setFocusPolicy (Qt::WheelFocus); // |= StrongFocus | TabFocus | ClickFocus
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
                    ? SIGNAL (columnsInserted (QModelIndex const&, int, int))
                    : SIGNAL (rowsInserted (QModelIndex const&, int, int))
                    , SLOT (sections_inserted (QModelIndex const&, int, int))
                    );
            connect ( model()
                    , orientation() == Qt::Horizontal
                    ? SIGNAL (columnsRemoved (QModelIndex const&, int, int))
                    : SIGNAL (rowsRemoved (QModelIndex const&, int, int))
                    , SLOT (sections_removed (QModelIndex const&, int, int))
                    );
            connect ( model()
                    , SIGNAL (headerDataChanged (Qt::Orientation, int, int))
                    , SLOT (data_changed (Qt::Orientation, int, int))
                    );

            _delegates.insert (0, model()->columnCount(), nullptr);
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
                                                  , QRect const& rect
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

        void delegating_header_view::wheelEvent (QWheelEvent* event)
        {
          const int section (logicalIndexAt (event->x()));
          if (section != -1 && delegate_for_section (section))
          {
            delegate_for_section (section)->wheel_event
              (section_index (this, section), event);
            return;
          }

          QHeaderView::wheelEvent (event);
        }

        bool delegating_header_view::event (QEvent* event)
        {
          //! \note These are enabled somewhere down the inheritance tree
          switch (event->type())
          {
          case QEvent::Wheel:
            wheelEvent (static_cast<QWheelEvent*> (event));
            return event->isAccepted();
          default:
            return QHeaderView::event (event);
          }
        }

        void delegating_header_view::sections_inserted
          (QModelIndex const&, int from, int to)
        {
          _delegates.insert (from, to - from + 1, nullptr);
          update();
        }

        void delegating_header_view::sections_removed
          (QModelIndex const&, int from, int to)
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
              = new widget::mini_button (QStyle::SP_TitleBarCloseButton, this);
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
              wid->style()->pixelMetric (QStyle::PM_HeaderGripMargin, nullptr, wid) + 1;
          }
        }

        ::boost::optional<int> delegating_header_view::current_editor() const
        {
          return _editor.section;
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
            _editor.widget = nullptr;

            delete _editor.close_button;
            _editor.close_button = nullptr;

            _editor.section = ::boost::none;

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

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

#include <util-qt/make_drop_target.hpp>

#include <util-qt/event_filter.hpp>

#include <QtCore/QMimeData>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDropEvent>
#include <QtWidgets/QWidget>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      void make_drop_target ( QWidget* widget
                            , std::function<bool (QMimeData const*)> check_accepted
                            , std::function<void (QDropEvent const*)> on_drop
                            )
      {
        widget->setAcceptDrops (true);

        util::qt::add_event_filter<QEvent::DragEnter, QEvent::Drop>
          ( widget
          , [check_accepted] (QDragEnterEvent* event)
            {
              if (check_accepted (event->mimeData()))
              {
                event->acceptProposedAction();
              }
              return false;
            }
          , [on_drop] (QDropEvent* event)
            {
              on_drop (event);
              return false;
            }
          );
      }

      void make_drop_target ( QWidget* widget
                            , QString const& accepted_mime_type
                            , std::function<void (QDropEvent const*)> on_drop
                            )
      {
        make_drop_target ( widget
                         , [accepted_mime_type] (QMimeData const* mime)
                           {
                             return mime->hasFormat (accepted_mime_type);
                           }
                         , std::move (on_drop)
                         );
      }
    }
  }
}

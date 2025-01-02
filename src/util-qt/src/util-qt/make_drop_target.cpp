// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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

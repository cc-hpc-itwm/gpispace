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

#include <util-qt/widget/mini_button.hpp>

#include <QtGui/QPainter>
#include <QtWidgets/QAction>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      namespace widget
      {
        mini_button::mini_button (QStyle::StandardPixmap icon, QWidget* parent)
          : QToolButton (parent)
        {
          auto* action (new QAction (this));
          action->setIcon (style()->standardIcon (icon));
          setDefaultAction (action);
        }
        mini_button::mini_button (QAction* action, QWidget* parent)
          : QToolButton (parent)
        {
          setDefaultAction (action);
        }

        QSize mini_button::sizeHint() const
        {
          if (icon().isNull())
          {
            return QToolButton::sizeHint();
          }

          ensurePolished();

          const int icon_size
            (style()->pixelMetric (QStyle::PM_SmallIconSize, nullptr, this));
          const QSize actual_icon_size
            (icon().actualSize (QSize (icon_size, icon_size)));
          const int side_length
            ( 2
            * style()->pixelMetric (QStyle::PM_DockWidgetTitleBarButtonMargin, nullptr, this)
            + qMax (actual_icon_size.width(), actual_icon_size.height())
            );
          return QSize (side_length, side_length);
        }
        QSize mini_button::minimumSizeHint() const
        {
          return sizeHint();
        }
      }
    }
  }
}

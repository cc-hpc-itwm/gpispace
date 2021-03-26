// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
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

#pragma once

#include <util-qt/widget/mini_button.fwd.hpp>

#include <QtWidgets/QStyle>
#include <QtWidgets/QToolButton>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      namespace widget
      {
        class mini_button : public QToolButton
        {
          Q_OBJECT

        public:
          mini_button (QStyle::StandardPixmap icon, QWidget* parent = nullptr);
          mini_button (QAction*, QWidget* parent = nullptr);

          virtual QSize sizeHint() const override;
          virtual QSize minimumSizeHint() const override;
        };
      }
    }
  }
}

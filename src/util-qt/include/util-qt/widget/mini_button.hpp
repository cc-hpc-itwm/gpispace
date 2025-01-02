// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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

          QSize sizeHint() const override;
          QSize minimumSizeHint() const override;
        };
      }
    }
  }
}

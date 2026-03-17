#pragma once

#include <gspc/util/qt/widget/mini_button.fwd.hpp>

#include <QtWidgets/QStyle>
#include <QtWidgets/QToolButton>




      namespace gspc::util::qt::widget
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

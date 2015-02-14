#pragma once

#include <QWidget>
#include <QPropertyAnimation>

class QTimer;

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      class popover_widget : public QWidget
      {
        Q_OBJECT

        public:
          //! \note the popover_widget takes ownership of content!
          popover_widget(QWidget* content);
          virtual QSize sizeHint() const override;

          QPoint arrowAdjustment() const;

        protected:
          virtual void paintEvent(QPaintEvent* event) override;
          virtual void resizeEvent(QResizeEvent* event) override;
          virtual void closeEvent(QCloseEvent* event) override;
          virtual void showEvent(QShowEvent* event) override;
          virtual void leaveEvent(QEvent* event) override;
          virtual void enterEvent(QEvent* event) override;

        public slots:
          void animationFinished();

        private:
          void createShape();
          QPropertyAnimation* createAnimation();

          const int _arrowOffset;
          const int _arrowLength;
          const int _roundness;
          const int _contentPadding;

          QWidget* _content;

          QRegion _widgetShape;
          QPainterPath _shapePath;
          QPropertyAnimation* _closingAnimation;
          QSize _wantedSize;

          QTimer* _hideTimer;
          int _hideTimeout;
      };
    }
  }
}

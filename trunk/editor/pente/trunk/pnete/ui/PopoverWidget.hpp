#ifndef UIPOPOVERWIDGET_HPP
#define UIPOPOVERWIDGET_HPP 1

#include <QWidget>

class QPropertyAnimation;
class QTimer;

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      class PopoverWidgetButton;
      class PopoverWidget : public QWidget
      {
        Q_OBJECT

        public:
          //! \note the PopoverWidget takes ownership of content!
          PopoverWidget(QWidget* content);
          QSize sizeHint() const;

          QPoint arrowAdjustment() const;

        protected:
          virtual void paintEvent(QPaintEvent* event);
          virtual void resizeEvent(QResizeEvent* event);
          virtual void closeEvent(QCloseEvent* event);
          virtual void showEvent(QShowEvent* event);
          virtual void leaveEvent(QEvent* event);
          virtual void enterEvent(QEvent* event);

        public slots:
          void animationFinished();

        private:
          void createShape();

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

#endif

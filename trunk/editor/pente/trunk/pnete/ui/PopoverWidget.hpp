#ifndef UIPOPOVERWIDGET_HPP
#define UIPOPOVERWIDGET_HPP 1

#include <QWidget>

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
          PopoverWidget(QWidget* attachTo, QWidget* content);
          QSize sizeHint() const;

        protected:
          void paintEvent(QPaintEvent* event);
          void resizeEvent(QResizeEvent* event);

        private:
          void createShape( const QSize& size );
          void createShapeWithArrowInSize( const QSize& size );

          const int _arrowOffset;
          const int _arrowLength;
          const int _roundness;
          const int _contentPadding;

          QWidget* _attachTo;
          QWidget* _content;

          QRegion _widgetShape;
          QPainterPath _shapePath;
          QSize _sizeHint;
      };
    }
  }
}

#endif

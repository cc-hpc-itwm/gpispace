#include "PopoverWidget.hpp"

#include <QPen>
#include <QBrush>
#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      PopoverWidget::PopoverWidget(QWidget* attachTo, QWidget* content)
        : QWidget( NULL, Qt::FramelessWindowHint | Qt::Popup )
        , _arrowOffset( 20 )
        , _arrowLength( 20 )
        , _roundness( 20 )
        , _contentPadding( 10 )
        , _attachTo( attachTo )
        , _content( content )
        {
          createShape( content->sizeHint() + QSize( _contentPadding, _contentPadding ) );
          content->setParent( this );
          connect(attachTo, SIGNAL(destroyed()), SLOT(deleteLater()));
        }

        void PopoverWidget::createShapeWithArrowInSize( const QSize& size )
        {
          createShape( size - QSize( _arrowLength, 0 ) );
        }

        void PopoverWidget::createShape( const QSize& size )
        {
          QPainterPath arrowPath;
          QPainterPath areaPath;

          QTransform rotate45Degrees;
          rotate45Degrees.rotate( 45 );

          QPolygon arrowPoly = rotate45Degrees.mapToPolygon( QRect( 0, 0, _arrowLength, _arrowLength ) );

          //! \todo which side? bottom and top as well?
          if( true /*left*/ )
          {
            arrowPoly.translate( _arrowLength, _arrowOffset );
            arrowPath.addPolygon( arrowPoly );
            areaPath.addRoundRect( QRect( QPoint( _arrowLength, 0 ), size ), _roundness );
            _content->move( _arrowLength + _contentPadding, _contentPadding );
          }
          else /*right*/
          {
            arrowPoly.translate( size.width(), _arrowOffset );
            arrowPath.addPolygon( arrowPoly );
            areaPath.addRoundRect( QRect( QPoint(), size ), _roundness );
            _content->move( _contentPadding, _contentPadding );
          }

          _shapePath = areaPath.united( arrowPath );
          _widgetShape = QRegion( _shapePath.toFillPolygon( ).toPolygon() );
          setMask( _widgetShape );
        }

        void PopoverWidget::paintEvent(QPaintEvent *)
        {
          static const QPen border( QBrush( Qt::black ), 4 );

          QPainter painter(this);
          painter.setRenderHint( QPainter::Antialiasing );
          painter.strokePath( _shapePath, border );
        }

        void PopoverWidget::resizeEvent( QResizeEvent* event )
        {
          createShapeWithArrowInSize( event->size() );
        }

        QSize PopoverWidget::sizeHint() const
        {
          return _widgetShape.boundingRect().size() + QSize( _arrowLength,0);
        }
    }
  }
}

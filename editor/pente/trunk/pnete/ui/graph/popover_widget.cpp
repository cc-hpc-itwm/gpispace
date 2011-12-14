#include <pnete/ui/graph/popover_widget.hpp>

#include <QPen>
#include <QBrush>
#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QCloseEvent>
#include <QPropertyAnimation>
#include <QTimer>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      popover_widget::popover_widget(QWidget* content)
        : QWidget( NULL, Qt::FramelessWindowHint | Qt::Popup )
        , _arrowOffset( 20 )
        , _arrowLength( 20 )
        , _roundness( 20 )
        , _contentPadding( 10 )
        , _content( content )
        , _closingAnimation(NULL)
        , _wantedSize(_content->minimumSize() + QSize(2 * _contentPadding, 2 * _contentPadding))
        , _hideTimer(new QTimer(this))
        , _hideTimeout(300)
        {
          _content->setParent(this);

          createShape();

          _hideTimer->setSingleShot(true);
          connect(_hideTimer, SIGNAL(timeout()), SLOT(close()));
        }

        QPoint popover_widget::arrowAdjustment() const
        {
          //! \todo other side?
          return QPoint(0.0, -_arrowOffset);
        }

        void popover_widget::enterEvent(QEvent* event)
        {
          _hideTimer->stop();
          if(_closingAnimation)
          {
            _closingAnimation->disconnect();
            int curTime = _closingAnimation->currentTime();
            _closingAnimation->setDirection(QPropertyAnimation::Forward);
            _closingAnimation->start();
            _closingAnimation->setCurrentTime(curTime);
            connect(_closingAnimation, SIGNAL(finished()), SLOT(animationFinished()));
          }
          QWidget::enterEvent(event);
        }

        void popover_widget::leaveEvent(QEvent* event)
        {
          _hideTimer->start(_hideTimeout);
          QWidget::leaveEvent(event);
        }

        void popover_widget::animationFinished()
        {
          _closingAnimation = NULL;
        }

        QPropertyAnimation* popover_widget::createAnimation()
        {
          QPropertyAnimation *animation = new QPropertyAnimation(this, "geometry");
          animation->setDuration(500);
          animation->setStartValue(QRectF(pos(), size() * 0.5));
          animation->setEndValue(QRectF(pos(), size()));
          animation->setEasingCurve(QEasingCurve::OutBack);
          return animation;
        }

        void popover_widget::showEvent(QShowEvent* event)
        {
          QPropertyAnimation *animation = createAnimation();

          connect(animation, SIGNAL(finished()), SLOT(animationFinished()));

          animation->start(QPropertyAnimation::DeleteWhenStopped);

          QWidget::showEvent(event);
        }

        void popover_widget::closeEvent(QCloseEvent* event)
        {
          if(_closingAnimation)
          {
            delete _closingAnimation;
            _closingAnimation = NULL;
            QWidget::closeEvent(event);
          }
          else
          {
            event->ignore();

            _closingAnimation = createAnimation();
            _closingAnimation->setDirection(QPropertyAnimation::Backward);

            connect(_closingAnimation, SIGNAL(finished()), SLOT(close()));
            connect(_closingAnimation, SIGNAL(finished()), SLOT(animationFinished()));

            _closingAnimation->start(QPropertyAnimation::DeleteWhenStopped);
          }
        }

        void popover_widget::createShape()
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
            areaPath.addRoundRect( QRect( QPoint( _arrowLength, 0 ), _wantedSize ), _roundness );
            _content->move( _arrowLength + _contentPadding, _contentPadding );
          }
          else /*right*/
          {
            arrowPoly.translate( _wantedSize.width(), _arrowOffset );
            arrowPath.addPolygon( arrowPoly );
            areaPath.addRoundRect( QRect( QPoint(), _wantedSize ), _roundness );
            _content->move( _contentPadding, _contentPadding );
          }

          _shapePath = areaPath.united( arrowPath );
          _widgetShape = QRegion( _shapePath.toFillPolygon( ).toPolygon() );
          setMask( _widgetShape );
        }

        void popover_widget::paintEvent(QPaintEvent *)
        {
          //QPixmap QPixmap::grabWidget ( QWidget * widget, const QRect & rectangle )
          static const QPen border( QBrush( Qt::black ), 4 );

          QPainter painter(this);
          painter.setRenderHint( QPainter::Antialiasing );
          painter.strokePath( _shapePath, border );
        }

        void popover_widget::resizeEvent( QResizeEvent* event )
        {
          _wantedSize = event->size() -  QSize(_arrowLength, 0);
          createShape();
        }

        QSize popover_widget::sizeHint() const
        {
          return _widgetShape.boundingRect().size() + QSize(_arrowLength,0);
        }
    }
  }
}

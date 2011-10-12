// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/ui/graph/transition.hpp>
#include <pnete/ui/graph/size.hpp>

#include <QPainter>
#include <QPen>
#include <QBrush>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        namespace transition
        {
          QPainterPath item::shape (const QSizeF& size) const
          {
            QPainterPath path;
            path.addRoundRect (bounding_rect (size), 20); // hardcoded constant
            return path;
          }
          QPainterPath item::shape () const
          {
            return shape (_size);
          }
          QRectF item::bounding_rect (const QSizeF& size) const
          {
            const QSizeF half_size (size / 2.0);
            return QRectF ( -half_size.width()
                          , -half_size.height()
                          , size.width()
                          , size.height()
                          );
          }
          QRectF item::boundingRect () const
          {
            return bounding_rect (_size);
          }

          void item::paint ( QPainter* painter
                           , const QStyleOptionGraphicsItem *option
                           , QWidget *widget
                           )
          {
            // hardcoded constants
            painter->setPen (QPen (QBrush ( isSelected()
                                          ? Qt::red
                                          : Qt::black
                                          )
                                  , 2.0
                                  )
                            );
            painter->setBackgroundMode (Qt::OpaqueMode);
            painter->setBrush (QBrush (Qt::white, Qt::SolidPattern));
            painter->drawPath (shape());

            painter->setPen (QPen (QBrush (Qt::black), 1.0));
            painter->setBackgroundMode (Qt::TransparentMode);

            QRectF rect (boundingRect());
            rect.setWidth (rect.width() - size::port::width());
            rect.setHeight (rect.height() - size::port::width());
            rect.translate ( size::port::height() / 2.0
                           , size::port::height() / 2.0
                           );

            painter->drawText ( rect
                              , Qt::AlignCenter | Qt::TextWordWrap
                              , name()
                              );
          }
        } // namespace transition
      }
    }
  }
}

// bernd.loerwald@itwm.fraunhofer.de

#ifndef _FHG_PNETE_UI_GRAPH_PLACE_HPP
#define _FHG_PNETE_UI_GRAPH_PLACE_HPP 1

#include <QPainter>
#include <QRectF>
#include <QStaticText>

class QWidget;
class QStyleOptionGraphicsItem;

#include <pnete/ui/graph_item.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        class place : public ::fhg::pnete::graph::ConnectableItem
        {
        public:
          place(::fhg::pnete::graph::graph_item* parent = NULL)
            : ::fhg::pnete::graph::ConnectableItem
                (ANYORIENTATION, ANYDIRECTION, parent)
            , _content()
            , _name (tr("<<a place>>"))
            , _we_type (tr("<<the type of a place>>"))
          {
            setFlag (QGraphicsItem::ItemIsMovable);

            refresh_content();
          }

          const QString& name (const QString& name_)
          {
            _name = name_;
            refresh_content();
            return _name;
          }
          const QString& we_type (const QString& we_type_)
          {
            _we_type = we_type_;
            refresh_content();
            return _we_type;
          }
          void refresh_content()
          {
            _content.setText (_name + " :: " + _we_type);
          }

          virtual QRectF boundingRect() const
          {
            const QSizeF half_size (_content.size() / 2.0);
            const QPointF pos (-half_size.width(), -half_size.height());
            return QRectF (pos, _content.size());
          }
          virtual void paint ( QPainter* painter
                             , const QStyleOptionGraphicsItem* option
                             , QWidget* widget = NULL
                             )
          {
            const QSizeF half_size (_content.size() / 2.0);
            const QPointF pos (-half_size.width(), -half_size.height());
            painter->drawStaticText (pos, _content);
            painter->drawRoundedRect( QRectF
                                      ( pos - QPointF (2.0, 2.0)
                                      , _content.size() + QSizeF (4.0, 4.0)
                                      )
                                    , 5.0
                                    , 5.0
                                    );
          }

          enum
          {
            Type = place_type,
          };
          virtual int type() const
          {
            return Type;
          }

        private:
          QStaticText _content;

          QString _name;
          QString _we_type;
        };
      }
    }
  }
}

#endif

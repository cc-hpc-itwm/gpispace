#ifndef GRAPHTRANSITION_HPP
#define GRAPHTRANSITION_HPP 1

#include <QGraphicsItem>
#include <QPainterPath>
#include <QRectF>
#include <QPointF>
#include <QSizeF>
#include <QObject>
#include <QMenu>

class QPainter;
class QStyleOptionGraphicsItem;
class QGraphicsSceneContextMenuEvent;
class QWidget;
class QAction;
class QMenu;

#include "data/Transition.hpp"
#include "graph_item.hpp"

namespace fhg
{
  namespace pnete
  {
    namespace graph
    {
      class Transition : public graph_item
      {
        Q_OBJECT
        Q_INTERFACES(QGraphicsItem)

        public:
          explicit Transition(const QString& title, const data::Transition& producedFrom, QGraphicsItem* parent = NULL);

          virtual QRectF boundingRect() const;
          virtual QPainterPath shape() const;

          const QString& title() const;
          bool highlighted() const;

          void repositionChildrenAndResize();

          const data::Transition& producedFrom() const;

          enum
          {
            Type = TransitionType,
          };
          virtual int type() const
          {
            return Type;
          }

          static Transition* create_from_library_data (const QByteArray& data);

        public slots:
          void slot_delete(void);
          void slot_add_port(void);

        protected:
          virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

          virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent* event);
          virtual void hoverEnterEvent(QGraphicsSceneHoverEvent* event);
          virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent* event);
          virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
          virtual void mousePressEvent(QGraphicsSceneMouseEvent* event);
          virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);

          //! \todo size verstellbar

          QPointF _dragStart;
          QSizeF _size;

          bool _highlighted;
          bool _dragging;

          data::Transition _producedFrom;

      private:
        QMenu _menu_context;

        void init_menu_context();

        QString _title;
      };
    }
  }
}

#endif

// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/ui/graph/base_item.hpp>

#include <pnete/data/handle/base.hpp>
#include <pnete/ui/graph/style/raster.hpp>
#include <util/qt/cast.hpp>

#include <fhg/util/backtracing_exception.hpp>

#include <boost/lexical_cast.hpp>

#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        base_item::base_item (base_item* parent)
          : QGraphicsObject (parent)
          , _style ()
          , _mode ()
          , _move_start ()
        {
          _mode.push (mode::NORMAL);
          setAcceptHoverEvents (true);
          setAcceptedMouseButtons (Qt::LeftButton);
        }

        scene_type* base_item::scene() const
        {
          QGraphicsScene* sc (QGraphicsItem::scene());

          return sc ? util::qt::throwing_qobject_cast<scene_type*> (sc)
                    : NULL
                    ;
        }

        void base_item::setPos (const QPointF& new_pos)
        {
          setPos (new_pos, false);
        }
        void base_item::setPos (const QPointF& new_pos, bool outer)
        {
          QPointF snapped (style::raster::snap (new_pos));

          handle().move (new_pos, outer);

          set_just_pos_but_not_in_property (snapped);
        }
        void base_item::setPos (qreal x, qreal y)
        {
          setPos (QPointF (x, y));
        }

        void base_item::no_undo_setPos (const QPointF& new_pos)
        {
          no_undo_no_raster_setPos (style::raster::snap (new_pos));
        }
        void base_item::no_undo_setPos (qreal x, qreal y)
        {
          no_undo_setPos (QPointF (x, y));
        }

        void base_item::no_undo_no_raster_setPos (const QPointF& new_pos)
        {
          handle().no_undo_move (new_pos);

          set_just_pos_but_not_in_property (new_pos);
        }
        void base_item::no_undo_no_raster_setPos (qreal x, qreal y)
        {
          no_undo_setPos (QPointF (x, y));
        }

        void base_item::set_just_pos_but_not_in_property (qreal x, qreal y)
        {
          set_just_pos_but_not_in_property (QPointF (x, y));
        }
        void base_item::set_just_pos_but_not_in_property (const QPointF& new_pos)
        {
          //! \todo update more clever
          foreach (base_item* child, childs())
            {
              child->setVisible (false);
            }

          QGraphicsItem::setPos (new_pos);

          foreach (base_item* child, childs())
            {
              child->setVisible (true);
            }
        }

        void base_item::clear_style_cache ()
        {
          _style.clear_cache();

          foreach (QGraphicsItem* child, childItems())
            {
              if (base_item* child_item = qgraphicsitem_cast<base_item*> (child))
                {
                  child_item->clear_style_cache();
                }
            }
        }

        void base_item::mode_push (const mode::type& mode)
        {
          _mode.push (mode);
          update ();
        }
        void base_item::mode_pop ()
        {
          _mode.pop();
          update ();
        }
        const mode::type& base_item::mode() const
        {
          if (_mode.empty())
            {
              throw std::runtime_error ("STRANGE: No mode!?");
            }

          return _mode.top();
        }

        void base_item::hoverEnterEvent(QGraphicsSceneHoverEvent * event)
        {
          mode_push (mode::HIGHLIGHT);
        }
        void base_item::hoverLeaveEvent(QGraphicsSceneHoverEvent * event)
        {
          mode_pop();
        }
        void base_item::mousePressEvent (QGraphicsSceneMouseEvent* event)
        {
          if (event->modifiers() == Qt::ControlModifier && is_movable())
            {
              mode_push (mode::MOVE);
              _move_start = event->pos();
            }
        }
        void base_item::mouseReleaseEvent (QGraphicsSceneMouseEvent* event)
        {
          if (mode() == mode::MOVE)
            {
              mode_pop();
            }
        }
        void base_item::mouseMoveEvent (QGraphicsSceneMouseEvent* event)
        {
          if (mode() == mode::MOVE)
            {
              setPos (pos() + event->pos() - _move_start);
            }
        }

        QLinkedList<base_item*> base_item::childs () const
        {
          QLinkedList<base_item*> childs;

          foreach (QGraphicsItem* childItem, childItems())
            {
              if (base_item* child = qgraphicsitem_cast<base_item *> (childItem))
                {
                  childs << child << child->childs();
                }
            }

          return childs;
        }

        QRectF base_item::boundingRect () const
        {
          return shape().controlPointRect();
        }

        const data::handle::base& base_item::handle() const
        {
          throw fhg::util::backtracing_exception
            ("base_item::handle() called: sub-class didn't define handle()");
        }

        void base_item::handle_property_change
          ( const ::we::type::property::key_type& key
          , const ::we::type::property::value_type& value
          )
        {
          const ::we::type::property::path_type path
            (we::type::property::util::split (key));

          if (path.size() > 1 && path[0] == "fhg" && path[1] == "pnete")
          {
            if (path.size() > 2 && path[2] == "position")
            {
              if (path.size() > 3)
              {
                if (path[3] == "x")
                {
                  set_just_pos_but_not_in_property
                    ( boost::lexical_cast<qreal>(value)
                    , pos().y()
                    );
                }
                else if (path[3] == "y")
                {
                  set_just_pos_but_not_in_property
                    ( pos().x()
                    , boost::lexical_cast<qreal>(value)
                    );
                }
              }
            }
          }
        }

        bool base_item::is_movable() const
        {
          return true;
        }
      }
    }
  }
}

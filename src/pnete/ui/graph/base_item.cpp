// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/ui/graph/base_item.hpp>

#include <pnete/data/handle/base.hpp>
#include <pnete/ui/graph/style/raster.hpp>
#include <util/qt/cast.hpp>

#include <we/type/value/path/split.hpp>

#include <fhg/util/backtracing_exception.hpp>
#include <fhg/util/num.hpp>
#include <fhg/util/parse/require.hpp>

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
                    : nullptr
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
          for (base_item* child : childs())
            {
              child->setVisible (false);
            }

          QGraphicsItem::setPos (new_pos);

          for (base_item* child : childs())
            {
              child->setVisible (true);
            }
        }

        void base_item::clear_style_cache ()
        {
          _style.clear_cache();

          for (QGraphicsItem* child : childItems())
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

        void base_item::hoverEnterEvent(QGraphicsSceneHoverEvent *)
        {
          mode_push (mode::HIGHLIGHT);
        }
        void base_item::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
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
        void base_item::mouseReleaseEvent (QGraphicsSceneMouseEvent*)
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

          for (QGraphicsItem* childItem : childItems())
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


        namespace
        {
          qreal read_qreal (const std::string& inp)
          {
            util::parse::position_string pos (inp);
            util::parse::require::skip_spaces (pos);
            return util::read_double (pos);
          }
        }

        void base_item::handle_property_change
          ( const ::we::type::property::key_type& key
          , const ::we::type::property::value_type& value
          )
        {
          const ::we::type::property::path_type path
            (pnet::type::value::path::split (key));
          ::we::type::property::path_type::const_iterator pos (path.begin());
          ::we::type::property::path_type::const_iterator const end (path.end());

          if (pos != end && *pos == "fhg")
          {
            ++pos;

            if (pos != end && *pos == "pnete")
            {
              ++pos;

              if (pos != end && *pos == "position")
              {
                ++pos;

                if (pos != end)
                {
                  if (*pos == "x")
                  {
                    set_just_pos_but_not_in_property
                      (read_qreal (value), this->pos().y());
                  }
                  else if (*pos == "y")
                  {
                    set_just_pos_but_not_in_property
                      (this->pos().x(), read_qreal (value));
                  }
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

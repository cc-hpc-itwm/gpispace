// mirko.rahn@itwm.fraunhofer.de

#include <pnete/ui/graph/style/cap.hpp>
#include <pnete/ui/graph/style/size.hpp>

#include <QPainterPath>
#include <QPolygon>
#include <QTransform>

#include <list>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        namespace cap
        {
          namespace data
          {
            class generic : public std::list<QPointF>
            {
            protected:
              void push (const qreal& x, const qreal& y)
              {
                push_back (QPointF (x, y));
              }

            public:
              generic () : std::list<QPointF>() {}
            };

            class out : public generic
            {
            public:
              out () : generic ()
              {
                push (0.0                , -(size::port::height() / 2.0));
                push (size::cap::length(),                          0.0 );
                push (0.0                ,  (size::port::height() / 2.0));
              }
            };

            class in : public generic
            {
            public:
              in () : generic ()
              {
                push (0.0                , -(size::port::height() / 2.0));
                push (size::cap::length(), -(size::port::height() / 2.0));
                push (0.0                ,                          0.0 );
                push (size::cap::length(),  (size::port::height() / 2.0));
                push (0.0                ,  (size::port::height() / 2.0));
              }
            };
          } // namespace data

          template<typename IT>
          void add ( QPainterPath* path
                   , const bool& middle
                   , const QPointF& offset
                   , const qreal& rotation
                   , IT pos
                   , const IT& end
                   )
          {
            QTransform transformation;
            transformation.translate (offset.x(), offset.y())
              .rotate (rotation);

            const qreal shift (middle ? (size::port::height() / 2.0) : 0.0);

            for (; pos != end; ++pos)
              {
                path->lineTo
                  (transformation.map (QPointF (pos->x(), pos->y() + shift)));
              }
          }

          void add_outgoing ( QPainterPath* path
                            , bool middle
                            , QPointF offset
                            , qreal rotation
                            )
          {
            static const data::out out;

            add (path, middle, offset, rotation, out.begin(), out.end());
          }

          void add_outgoing ( QPolygonF* poly
                            , QPointF offset
                            , qreal rotation
                            )
          {
            QPainterPath path;
            add_outgoing (&path, false, offset, rotation);
            *poly = poly->united (path.toFillPolygon());
          }

          void add_incoming ( QPainterPath* path
                            , bool middle
                            , QPointF offset
                            , qreal rotation
                            )
          {
            static const data::in in;

            add (path, middle, offset, rotation, in.begin(), in.end());
          }

          void add_incoming ( QPolygonF* poly
                            , QPointF offset
                            , qreal rotation
                            )
          {
            QPainterPath path;
            add_incoming (&path, false, offset, rotation);
            *poly = poly->united (path.toFillPolygon());
          }
        } // namespace cap
      }
    }
  }
}

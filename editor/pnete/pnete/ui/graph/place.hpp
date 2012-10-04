// bernd.loerwald@itwm.fraunhofer.de

#ifndef _FHG_PNETE_UI_GRAPH_PLACE_HPP
#define _FHG_PNETE_UI_GRAPH_PLACE_HPP 1

#include <QPainter>
#include <QRectF>
#include <QStaticText>
#include <QPointF>
#include <QSizeF>

#include <boost/optional.hpp>

#include <xml/parse/types.hpp>

#include <pnete/weaver/weaver.hpp>

class QWidget;
class QStyleOptionGraphicsItem;

#include <pnete/ui/graph/connectable_item.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        namespace place
        {
          typedef ITVAL(XMLTYPE(net_type::places_type)) place_type;

          class item : public graph::connectable::item
          {
            Q_OBJECT;

          public:
            item
            ( place_type& place
            , boost::optional< ::xml::parse::type::type_map_type&> type_map
            = boost::none
            , graph::item* parent = NULL
            );

            const place_type& place () const;

            const std::string& name() const;
            const std::string& we_type() const;

            virtual void setPos (const QPointF&);
            virtual QPainterPath shape() const;

          public slots:
            void refresh_content();

          public:
            virtual void paint ( QPainter* painter
                               , const QStyleOptionGraphicsItem* option
                               , QWidget* widget = NULL
                               );

            enum { Type = place_graph_type };
            virtual int type() const { return Type; }

          private:
            place_type& _place;

            QStaticText _content;

            const QStaticText& content() const;
            QSizeF content_size() const;
            QPointF content_pos() const;
          };
        }
      }
    }
  }
}

#endif

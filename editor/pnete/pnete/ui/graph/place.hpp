// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_PNETE_UI_GRAPH_PLACE_HPP
#define FHG_PNETE_UI_GRAPH_PLACE_HPP

#include <pnete/ui/graph/place.fwd.hpp>

#include <pnete/data/handle/place.hpp>
#include <pnete/ui/graph/connectable_item.hpp>

#include <string>

#include <boost/optional.hpp>

#include <QStaticText>
#include <QPointF>
#include <QSizeF>

class QPainter;
class QWidget;
class QStyleOptionGraphicsItem;

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        class place_item : public connectable_item
        {
          Q_OBJECT;

        public:
          place_item
            ( const data::handle::place& handle
            , base_item* parent = NULL
            );

          virtual const data::handle::place& handle() const;

          std::string name() const;
          const std::string& we_type() const;

          virtual void setPos (const QPointF&);
          virtual QPainterPath shape() const;

        public slots:
          void refresh_content();

          void property_changed
            ( const QObject* origin
            , const data::handle::place& changed_handle
            , const ::we::type::property::key_type& key
            , const ::we::type::property::value_type& value
            );

          void slot_association_added (association* c);
          void slot_association_removed (association* c);
          void association_changed_in_any_way();

        public:
          virtual void paint ( QPainter* painter
                             , const QStyleOptionGraphicsItem* option
                             , QWidget* widget = NULL
                             );

          enum { Type = place_graph_type };
          virtual int type() const { return Type; }

        private:
          data::handle::place _handle;

          QStaticText _content;

          const QStaticText& content() const;
          QSizeF content_size() const;
          QPointF content_pos() const;
        };
      }
    }
  }
}

#endif

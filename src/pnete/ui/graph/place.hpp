// bernd.loerwald@itwm.fraunhofer.de

#pragma once

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
          Q_OBJECT

        public:
          place_item
            ( const data::handle::place& handle
            , base_item* parent = nullptr
            );

          virtual const data::handle::place& handle() const override;

          virtual bool is_connectable_with (const connectable_item*) const override;

          std::string name() const;
          virtual const std::string& we_type() const override;

          virtual void setPos (const QPointF&) override;
          virtual QPainterPath shape() const override;

          virtual bool is_movable() const override;

        public slots:
          void refresh_content();

          void place_deleted (const data::handle::place&);

          void type_or_name_changed
            (const data::handle::place&, const QString&);

          void property_changed
            ( const data::handle::place&
            , const we::type::property::path_type&
            , const we::type::property::value_type&
            );

          void place_is_virtual_changed (const data::handle::place&, bool);

          void slot_association_added (association* c);
          void slot_association_removed (association* c);
          void association_changed_in_any_way();

        public:
          virtual void paint ( QPainter* painter
                             , const QStyleOptionGraphicsItem* option
                             , QWidget* widget = nullptr
                             ) override;

          enum { Type = place_graph_type };
          virtual int type() const override { return Type; }

        private:
          data::handle::place _handle;

          QStaticText _content;

          const QStaticText& content() const;
          QSizeF content_size() const;
          QPointF content_pos() const;
          void update_implicity();
        };
      }
    }
  }
}

// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/ui/graph/style/isc13.hpp>

#include <QMap>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace graph
      {
        namespace style
        {
          namespace isc13
          {
            namespace
            {
              QMap<QString, QBrush> create_c_f_t (qreal factor)
              {
                QMap<QString, QBrush> map;


                map["velocity_type"] = QColor::fromHsvF (0.166667, 0.975799, qMin (1.0, 1.0 * factor));
                map["data_type"] = QColor::fromHsvF (0.58, 1.0, qMin (1.0, 1.0 * factor));

                return map;
              }
            }

            void add_colors_for_types ( style::type* style
                                      , const boost::function
                                        < boost::optional<QBrush>
                                          ( const base_item*
                                          , const QBrush&
                                          , const QString&
                                          )>& fun
                                      )
            {
              static QMap<QString, QBrush> normal (create_c_f_t (0.8));
              static QMap<QString, QBrush> highlight (create_c_f_t (1.0));
              static QMap<QString, QBrush> move (create_c_f_t (0.6));

              foreach (const QString& type, normal.keys())
              {
                style->push<QBrush> ( "background_brush"
                                    , mode::NORMAL
                                    , boost::bind ( fun
                                                  , _1
                                                  , normal[type]
                                                  , type
                                                  )
                                    );
                style->push<QBrush> ( "background_brush"
                                    , mode::HIGHLIGHT
                                    , boost::bind ( fun
                                                  , _1
                                                  , highlight[type]
                                                  , type
                                                  )
                                    );
                style->push<QBrush> ( "background_brush"
                                    , mode::MOVE
                                    , boost::bind ( fun
                                                  , _1
                                                  , move[type]
                                                  , type
                                                  )
                                    );
              }
            }
          }
        }
      }
    }
  }
}

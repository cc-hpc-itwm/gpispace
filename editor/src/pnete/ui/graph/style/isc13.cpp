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
              QMap<QString, QColor> create_c_f_t()
              {
                QMap<QString, QColor> map;
                map["data_type"] = Qt::yellow;
                map["file_type"] = Qt::blue;
                return map;
              }

              const QMap<QString, QColor>& colors_for_types()
              {
                static QMap<QString, QColor> map (create_c_f_t());
                return map;
              }

            }

            void add_colors_for_types ( style::type* style
                                      , const boost::function
                                        < boost::optional<QColor>
                                          ( const base_item*
                                          , const QColor&
                                          , const QString&
                                          )>& fun
                                      )
            {
              foreach (const QString& type, colors_for_types().keys())
              {
                style->push<QColor> ( "background_color"
                                    , mode::NORMAL
                                    , boost::bind ( fun
                                                  , _1
                                                  , colors_for_types()[type]
                                                  , type
                                                  )
                                    );
                style->push<QColor> ( "background_color"
                                    , mode::HIGHLIGHT
                                    , boost::bind ( fun
                                                  , _1
                                                  , colors_for_types()[type]
                                                  .lighter()
                                                  , type
                                                  )
                                    );
                style->push<QColor> ( "background_color"
                                    , mode::MOVE
                                    , boost::bind ( fun
                                                  , _1
                                                  , colors_for_types()[type]
                                                  .darker()
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

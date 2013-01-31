// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/ui/port_lists_widget.hpp>

#include <pnete/ui/ComboItemDelegate.hpp>

#include <xml/parse/type/function.hpp>

#include <boost/foreach.hpp>
#include <boost/range/adaptor/filtered.hpp>

#include <QGroupBox>
#include <QHeaderView>
#include <QObject>
#include <QSplitter>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QWidget>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      namespace detail
      {
        namespace
        {
          bool same_direction ( const ::xml::parse::id::ref::port& id
                              , const we::type::PortDirection& direction
                              )
          {
            return id.get().direction() == direction;
          }
          data::handle::port make_handle ( const ::xml::parse::id::ref::port& id
                                         , const data::handle::function& function
                                         )
          {
            return data::handle::port (id, function.document());
          }
        }

        ports_model::ports_model ( const data::handle::function& function
                                 , const we::type::PortDirection& direction
                                 , QObject* parent
                                 )
          : QAbstractTableModel (parent)
          , _function (function)
          , _direction (direction)
        {
          function.connect_to_change_mgr
            (this, "port_added", "data::handle::port");


          BOOST_FOREACH ( const data::handle::port& handle
                        , function.get().ports().ids()
                        | boost::adaptors::filtered
                          (boost::bind (same_direction, _1, _direction))
                        | boost::adaptors::transformed
                          (boost::bind (make_handle, _1, _function))
                        )
          {
            _ports << handle;

            handle.connect_to_change_mgr
              ( this
              , "name_set", "type_or_name_changed"
              , "data::handle::port, QString"
              );
            handle.connect_to_change_mgr
              ( this
              , "type_set", "type_or_name_changed"
              , "data::handle::port, QString"
              );

            handle.connect_to_change_mgr
              (this, "port_deleted", "data::handle::port");
          }
        }

        void ports_model::port_added
          (const QObject*, const data::handle::port& handle)
        {
          if ( handle.get().parent()->id() == _function.id()
             && same_direction (handle.id(), _direction)
             )
          {
            _ports << handle;
            reset();
          }
        }

        void ports_model::port_deleted
          (const QObject*, const data::handle::port& changed)
        {
          if (_ports.contains (changed))
          {
            _ports.removeAll (changed);
            reset();
          }
        }

        void ports_model::type_or_name_changed
          (const QObject*, const data::handle::port& changed, const QString&)
        {
          if (_ports.contains (changed))
          {
            reset();
          }
        }

        int ports_model::rowCount (const QModelIndex&) const
        {
          return _ports.size();
        }

        int ports_model::columnCount (const QModelIndex&) const
        {
          return 2;
        }

        QVariant ports_model::data (const QModelIndex& index, int role) const
        {
          if (index.isValid() && role == Qt::DisplayRole)
          {
            switch (index.column())
            {
            case 0:
              return QString::fromStdString
                (_ports.at (index.row()).get().name());
            case 1:
              return QString::fromStdString
                (_ports.at (index.row()).get().type());
            }
          }

          return QVariant();
        }

        QVariant ports_model::headerData ( int section
                                         , Qt::Orientation orientation
                                         , int role
                                         ) const
        {
          if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
          {
            switch (section)
            {
            case 0:
              return tr ("port_name");
            case 1:
              return tr ("port_type");
            }
          }

          return QVariant();
        }
      }

      namespace
      {
        QGroupBox* port_list_box
          ( const we::type::PortDirection& which
          , const data::handle::function& function
          , const QStringList& types
          )
        {
          //! \todo make this nicer
          //! \todo add edit facilities
          //! \todo adjust column sizes automatically
          QTableView* table (new QTableView);

          table->setModel (new detail::ports_model (function, which, table));
          table->setItemDelegateForColumn
            (1, new ComboBoxItemDelegate (types, table));
          table->verticalHeader()->hide();
          table->resizeRowsToContents();
          table->resizeColumnsToContents();
          table->setSelectionMode (QAbstractItemView::NoSelection);

          QGroupBox* group_box
            ( new QGroupBox ( QObject::tr ( which == we::type::PORT_IN
                                          ? "in_ports_header"
                                          : ( which == we::type::PORT_OUT
                                            ? "out_ports_header"
                                            : "tunnel_ports_header"
                                            )
                                          )
                            )
            );
          QVBoxLayout* vbox (new QVBoxLayout (group_box));
          vbox->addWidget (table);
          group_box->setLayout (vbox);
          return group_box;
        }
      }

      port_lists_widget::port_lists_widget
        ( const data::handle::function& function
        , const QStringList& types
        , QWidget* parent
        )
          : QWidget (parent)
      {
        QSplitter* splitter (new QSplitter (Qt::Vertical));

        splitter->addWidget
          (port_list_box (we::type::PORT_IN, function, types));
        splitter->addWidget
          (port_list_box (we::type::PORT_OUT, function, types));
        splitter->addWidget
          (port_list_box (we::type::PORT_TUNNEL, function, types));

        QVBoxLayout* vbox (new QVBoxLayout());
        vbox->addWidget (splitter);
        vbox->setContentsMargins (0, 0, 0, 0);
        setLayout(vbox);
      }
    }
  }
}

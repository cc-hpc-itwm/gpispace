// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/ui/port_lists_widget.hpp>

#include <pnete/ui/ComboItemDelegate.hpp>

#include <util/qt/boost_connect.hpp>

#include <xml/parse/type/function.hpp>

#include <boost/bind.hpp>

#include <QAction>
#include <QGroupBox>
#include <QHeaderView>
#include <QObject>
#include <QSplitter>
#include <QTableWidget>
#include <QToolBar>
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
        ports_model::ports_model ( const data::handle::function& function
                                 , const we::type::PortDirection& direction
                                 , QObject* parent
                                 )
          : QAbstractTableModel (parent)
          , _function (function)
          , _direction (direction)
          , _ports (function.ports (_direction))
        {
          function.connect_to_change_mgr
            (this, "port_added", "data::handle::port");

          function.connect_to_change_mgr
            ( this
            , "name_set", "type_or_name_changed"
            , "data::handle::port, QString"
            );
          function.connect_to_change_mgr
            ( this
            , "type_set", "type_or_name_changed"
            , "data::handle::port, QString"
            );

          function.connect_to_change_mgr
            (this, "port_deleted", "data::handle::port");
        }

        void ports_model::port_added (const data::handle::port& handle)
        {
          if (handle.parent_is (_function) && handle.direction_is (_direction))
          {
            _ports << handle;
            reset();
          }
        }

        void ports_model::port_deleted (const data::handle::port& changed)
        {
          if (_ports.contains (changed))
          {
            _ports.removeAll (changed);
            reset();
          }
        }

        void ports_model::type_or_name_changed
          (const data::handle::port& changed, const QString&)
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
        Qt::ItemFlags ports_model::flags (const QModelIndex& index) const
        {
          if (!index.isValid())
          {
            return Qt::ItemIsEnabled;
          }

          return QAbstractTableModel::flags (index) | Qt::ItemIsEditable;
        }


        QVariant ports_model::data (const QModelIndex& index, int role) const
        {
          if ( index.isValid()
             && (role == Qt::DisplayRole || role == Qt::EditRole)
             )
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

        bool ports_model::setData ( const QModelIndex& index
                                  , const QVariant& value
                                  , int role
                                  )
        {
          if (!index.isValid() || role != Qt::EditRole)
          {
            return false;
          }

          switch (index.column())
          {
          case 0:
            _ports.at (index.row()).set_name (value.toString());
            break;
          case 1:
            _ports.at (index.row()).set_type (value.toString());
            break;
          default:
            return false;
          }

          emit dataChanged (index, index);
          return true;
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

        if (function.content_is_net())
        {
          splitter->addWidget
            (port_list_box (we::type::PORT_TUNNEL, function, types));
        }

        QVBoxLayout* vbox (new QVBoxLayout());

        QToolBar* bar (new QToolBar (QObject::tr ("dummy"), this));

        fhg::util::qt::boost_connect<void()>
          ( bar->addAction (QObject::tr ("add_in_port"))
          , SIGNAL (triggered())
          , this
          , boost::bind ( &data::handle::function::add_port
                        , function
                        , we::type::PORT_IN
                        , boost::none
                        )
          );

        fhg::util::qt::boost_connect<void()>
          ( bar->addAction (QObject::tr ("add_out_port"))
          , SIGNAL (triggered())
          , this
          , boost::bind ( &data::handle::function::add_port
                        , function
                        , we::type::PORT_OUT
                        , boost::none
                        )
          );

        if (function.content_is_net())
        {
          fhg::util::qt::boost_connect<void()>
            ( bar->addAction (QObject::tr ("add_tunnel_port"))
            , SIGNAL (triggered())
            , this
            , boost::bind ( &data::handle::function::add_port
                          , function
                          , we::type::PORT_TUNNEL
                          , boost::none
                          )
            );
        }

        vbox->addWidget (bar);
        vbox->addWidget (splitter);
        vbox->setContentsMargins (0, 0, 0, 0);
        setLayout (vbox);
      }
    }
  }
}

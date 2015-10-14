// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#pragma once

#include <pnete/data/handle/function.hpp>
#include <pnete/data/handle/port.hpp>

#include <QAbstractTableModel>
#include <QList>
#include <QObject>
#include <QWidget>

class QStringList;
class QWidget;

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      //! \note Only exposed as it needs to be moced.
      namespace detail
      {
       class ports_model : public QAbstractTableModel
        {
          Q_OBJECT

        public:
          ports_model ( const data::handle::function&
                      , const we::type::PortDirection&
                      , QObject* parent = nullptr
                      );

          virtual int rowCount (const QModelIndex& = QModelIndex()) const override;
          virtual int columnCount (const QModelIndex& = QModelIndex()) const override;
          virtual Qt::ItemFlags flags (const QModelIndex&) const override;

          virtual QVariant data (const QModelIndex&, int = Qt::DisplayRole) const override;
          virtual QVariant headerData (int, Qt::Orientation, int = Qt::DisplayRole) const override;

          virtual bool setData (const QModelIndex&, const QVariant&, int = Qt::EditRole) override;

        public slots:
          void port_added (const data::handle::port&);
          void type_or_name_changed (const data::handle::port&, const QString&);
          void port_deleted (const data::handle::port&);

        private:
          data::handle::function _function;
          we::type::PortDirection _direction;
          QList<data::handle::port> _ports;
        };
      }

      class port_lists_widget : public QWidget
      {
        Q_OBJECT

      public:
        explicit port_lists_widget ( const data::handle::function&
                                   , const QStringList& types
                                   , QWidget* parent = nullptr
                                   );
      };
    }
  }
}

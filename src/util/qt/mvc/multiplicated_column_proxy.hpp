#pragma once

#include <util/qt/mvc/fixed_proxy_models.hpp>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      namespace mvc
      {
        class multiplicated_column_proxy : public id_proxy
        {
          Q_OBJECT

        public:
          multiplicated_column_proxy (QAbstractItemModel*, QObject* parent = nullptr);

          virtual int columnCount (const QModelIndex& = QModelIndex()) const override;
          virtual QModelIndex mapToSource (const QModelIndex& proxy) const override;
          virtual bool insertColumns
            (int column, int count, const QModelIndex& parent = QModelIndex()) override;
          virtual bool removeColumns
            (int column, int count, const QModelIndex& parent = QModelIndex()) override;

        private slots:
          void source_dataChanged (const QModelIndex&, const QModelIndex&);

        private:
          int _column_count;
        };
      }
    }
  }
}

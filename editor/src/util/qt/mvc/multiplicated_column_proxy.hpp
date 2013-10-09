// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_UTIL_QT_MVC_MULTIPLICATED_COLUMN_PROXY_HPP
#define FHG_UTIL_QT_MVC_MULTIPLICATED_COLUMN_PROXY_HPP

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
          multiplicated_column_proxy (QAbstractItemModel*, QObject* parent = NULL);

          virtual int columnCount (const QModelIndex& = QModelIndex()) const;
          virtual QModelIndex mapToSource (const QModelIndex& proxy) const;
          virtual bool insertColumns
            (int column, int count, const QModelIndex& parent = QModelIndex());
          virtual bool removeColumns
            (int column, int count, const QModelIndex& parent = QModelIndex());

        private slots:
          void source_dataChanged (const QModelIndex&, const QModelIndex&);

        private:
          int _column_count;
        };
      }
    }
  }
}

#endif

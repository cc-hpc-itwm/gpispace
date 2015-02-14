// bernd.loerwald@itwm.fraunhofer.de

#pragma once

#include <pnete/data/manager.fwd.hpp>

#include <QAbstractItemModel>
#include <QObject>

class QWidget;
class QModelIndex;
class QMimeData;
class QDir;
class QFileSystemWatcher;

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      class TransitionLibraryItem;

      class TransitionLibraryModel : public QAbstractItemModel
      {
        Q_OBJECT

        public:
          TransitionLibraryModel (data::manager&, QWidget* parent = nullptr);

          static const QString mimeType;

          virtual QModelIndex index ( int row
                                    , int column
                                    , const QModelIndex& parent = QModelIndex()
                                    ) const override;
          virtual QModelIndex parent (const QModelIndex& index) const override;
          virtual int rowCount (const QModelIndex& parent = QModelIndex()) const override;
          virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;
          virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
          virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
          virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
          virtual QMimeData * mimeData(const QModelIndexList& indexes) const override;

          void addContentFromDirectory(const QString& path, bool trusted = false);

        public slots:
          void rereadAllDirectories(const QString& /*path*/);

        private:
          void setFileSystemWatcher(const QString& path);
          void readContentFromDirectoryRecursive(TransitionLibraryItem* currentRoot, const bool& trusted, const QString& path);

          data::manager& _data_manager;

          QFileSystemWatcher* _fileSystemWatcher;
          TransitionLibraryItem* _items;

          QList<QString> _trustedPaths;
          QList<QString> _untrustedPaths;
      };
    }
  }
}

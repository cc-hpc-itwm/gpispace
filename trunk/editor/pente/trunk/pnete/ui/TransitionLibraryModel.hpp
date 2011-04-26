#ifndef UITRANSITIONLIBRARYMODEL_HPP
#define UITRANSITIONLIBRARYMODEL_HPP 1

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
          TransitionLibraryModel(const QDir& path, QWidget* parent = NULL);
          
          static const QString mimeType;
          
          virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
          virtual QModelIndex parent(const QModelIndex& index) const;
          virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
          virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
          virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
          virtual Qt::ItemFlags flags(const QModelIndex& index) const;
          virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
          virtual QMimeData * mimeData(const QModelIndexList& indexes) const;
        
        public slots:
          void readContentFromDirectory(const QString& path);
          
        private:
          void setFileSystemWatcher(const QString& path);
          void readContentFromDirectoryRecursive(TransitionLibraryItem* currentRoot, const QString& path, bool topLevel);
          
          QFileSystemWatcher* _fileSystemWatcher;
          TransitionLibraryItem* _items;
      };
    }
  }
}

#endif

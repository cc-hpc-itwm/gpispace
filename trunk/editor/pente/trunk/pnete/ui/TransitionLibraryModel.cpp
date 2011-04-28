#include "TransitionLibraryModel.hpp"
#include "TransitionLibraryItem.hpp"

#include "data/Transition.hpp"

#include <QWidget>
#include <QModelIndex>
#include <QMimeData>
#include <QDir>
#include <QFileSystemWatcher>
#include <QDirIterator>
#include <QIcon>
#include <QCoreApplication>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      const QString TransitionLibraryModel::mimeType = "pnete/transition";
      
      TransitionLibraryModel::TransitionLibraryModel(const QDir& path, QWidget* parent)
      : QAbstractItemModel(parent),
      _fileSystemWatcher(NULL),
      _items(new TransitionLibraryItem("__DUMMY__ROOT__", true, this))
      {
        readContentFromDirectory(path.path());
      }
      
      void TransitionLibraryModel::readContentFromDirectory(const QString& path)
      {
        _items->clearChildren();
 
        readContentFromDirectoryRecursive(_items, true, path);
        _items->sortChildren();
      }
      
      void TransitionLibraryModel::addContentFromDirectory(const QString& path, bool trusted)
      {
        readContentFromDirectoryRecursive(_items, trusted, path);
        _items->sortChildren();
      }
      
      void TransitionLibraryModel::readContentFromDirectoryRecursive(TransitionLibraryItem* currentRoot, const bool& trusted, const QString& path)
      {
        setFileSystemWatcher(path);
        
        QDirIterator directoryWalkerDirs(path, QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);
        while(directoryWalkerDirs.hasNext())
        {
          directoryWalkerDirs.next();
          
          const QString newName = directoryWalkerDirs.fileInfo().fileName();
          
          TransitionLibraryItem* newRoot;
          bool found = false;
          const QList<TransitionLibraryItem*>& children = currentRoot->children();
          for(QList<TransitionLibraryItem*>::const_iterator it = children.begin(); it != children.end(); ++it)
          {
            if(!(*it)->data() && (*it)->name() == newName)
            {
              newRoot = *it;
              found = true;
              break;
            }
          }
          if(!found)
          {
            newRoot = new TransitionLibraryItem(newName, trusted, currentRoot);
            currentRoot->appendChild(newRoot);
          }
          readContentFromDirectoryRecursive(newRoot, trusted, directoryWalkerDirs.fileInfo().absoluteFilePath());
        }
        
        QDirIterator directoryWalkerFiles(path, QStringList("*.xml"), QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot);
        while(directoryWalkerFiles.hasNext())
        {
          directoryWalkerFiles.next();
          
          data::Transition* transition = new data::Transition(directoryWalkerFiles.fileInfo().absoluteFilePath());
          currentRoot->appendChild(new TransitionLibraryItem(transition, trusted, currentRoot));
        }
      }
      
      void TransitionLibraryModel::setFileSystemWatcher(const QString& path)
      {
        //! \todo fix this to be working with more than one folder! (i.e. rescan all directories?)
        return;
        
        //delete _fileSystemWatcher;
        
        //_fileSystemWatcher = new QFileSystemWatcher(QStringList(path), this);
        //connect(_fileSystemWatcher, SIGNAL(directoryChanged(const QString&)), SLOT(readContentFromDirectory(const QString&)));
      }
      
      int TransitionLibraryModel::rowCount(const QModelIndex& parent) const
      {
        if(parent.column() > 0)
        {
          return 0;
        }
    
        TransitionLibraryItem* parentItem = !parent.isValid() ? _items : static_cast<TransitionLibraryItem*>(parent.internalPointer());
        return parentItem->childCount();
      }
      
      int TransitionLibraryModel::columnCount(const QModelIndex& parent) const
      {
        return 2;
      }
      
      QVariant TransitionLibraryModel::data(const QModelIndex& index, int role) const
      {
        //! \todo symbols for trusted and untrusted entries? (lib / user)
        if(index.isValid())
        {
          TransitionLibraryItem* item = static_cast<TransitionLibraryItem*>(index.internalPointer());
          switch(role)
          {
            case Qt::DisplayRole:
              if(index.column() == 0)
              {
                return item->name();
              }
              break;
              
            case Qt::DecorationRole:
            //! \todo folder icon?
              if(index.column() == 1 && item->data() && item->trusted())
              {
                return QIcon(":/lock.png");
              }
              break;
              
            case Qt::ToolTipRole:
              if(index.column() == 1 && item->data() && item->trusted())
              {
                return tr("Trusted Transition");
              }
              break;
              
            default:
              ;
          }
        }
        return QVariant();
      }
      
      QVariant TransitionLibraryModel::headerData(int section, Qt::Orientation orientation, int role) const
      {
        if(orientation != Qt::Horizontal || role != Qt::DisplayRole || section != 0)
        {
          return QVariant();
        }
        else
        {
          return QString(tr("Transition"));
        }
      }
      
      QMimeData* TransitionLibraryModel::mimeData(const QModelIndexList& indexes) const
      {
        //! \todo multiple at once!
        
        TransitionLibraryItem* item = static_cast<TransitionLibraryItem*>(indexes.first().internalPointer());
        if(item->data())
        {
          QMimeData* mimeData = new QMimeData;
          QByteArray byteArray;
          // Thanks Qt for providing a "toByteArray()" method.
          QDataStream stream(&byteArray, QIODevice::WriteOnly);
          stream << *item->data();
          mimeData->setData(mimeType, byteArray);
          
          return mimeData;
        }
        else
        {
          return NULL;
        }
      }
      
      QModelIndex TransitionLibraryModel::index(int row, int column, const QModelIndex& parent) const
      {
        if(hasIndex(row, column, parent))
        {
          TransitionLibraryItem* parentItem = !parent.isValid() ? _items : static_cast<TransitionLibraryItem*>(parent.internalPointer());
    
          TransitionLibraryItem* childItem = parentItem->child(row);
          if(childItem)
          {
            return createIndex(row, column, childItem);
          }
        }
        
        return QModelIndex();
      }
      
      QModelIndex TransitionLibraryModel::parent(const QModelIndex& index) const
      {
        if (!index.isValid())
        {
          return QModelIndex();
        }
    
        TransitionLibraryItem* childItem = static_cast<TransitionLibraryItem*>(index.internalPointer());
        TransitionLibraryItem* parentItem = childItem->parent();
    
        if(parentItem == _items)
        {
          return QModelIndex();
        }
        else
        {
          return createIndex(parentItem->row(), 0, parentItem);
        }
      }
      
      Qt::ItemFlags TransitionLibraryModel::flags(const QModelIndex& index) const
      {
        if (!index.isValid())
        {
          return Qt::NoItemFlags;
        }
        else
        {
          return Qt::ItemIsDragEnabled | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
        }
      }
    }
  }
}

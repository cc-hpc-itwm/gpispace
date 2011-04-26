#ifndef UITRANSITIONLIBRARYITEM_HPP
#define UITRANSITIONLIBRARYITEM_HPP 1

#include <QList>
#include <QObject>
#include <QVariant>

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      class Transition;
    }
    namespace ui
    {
      class TransitionLibraryItem : public QObject
      {
        Q_OBJECT
        
        public:
          TransitionLibraryItem(data::Transition* data, QObject* parent = NULL);
          TransitionLibraryItem(const QString& name, QObject* parent = NULL);
          
          void appendChild(TransitionLibraryItem* child);
          
          TransitionLibraryItem* child(int row) const;
          int childCount() const;
          data::Transition* data() const;
          const QString& name() const;
          int row() const;
          TransitionLibraryItem* parent() const;
          
          void clearChildren();
        
        private:
          data::Transition* _data;
          QString _name;
          
          QList<TransitionLibraryItem*> _children;
          TransitionLibraryItem* _parent;
      };
    }
  }
}

#endif

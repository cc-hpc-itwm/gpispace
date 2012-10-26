// bernd.loerwald@itwm.fraunhofer.de

#ifndef _PNETE_UTIL_HPP
#define _PNETE_UTIL_HPP 1

#include <boost/shared_ptr.hpp>

#include <QWidget>
#include <QObject>
#include <QGraphicsItem>

#include <stdexcept>

namespace fhg
{
  namespace util
  {
    template<typename T>
    inline T*
    first_parent_being_a (QWidget* widget)
    {
      while (widget && !qobject_cast<T*> (widget))
      {
        widget = widget->parentWidget();
      }
      return qobject_cast<T*> (widget);
    }
    template<typename T>
    inline T*
    first_parent_being_a (QObject* object)
    {
      while (object && !qobject_cast<T*> (object))
      {
        object = object->parent();
      }
      return qobject_cast<T*> (object);
    }
    template<class T>
    class ptr_hasher
    {
    public:
      std::size_t operator () (const boost::shared_ptr<T>& key) const
      {
        return reinterpret_cast<std::size_t> (key.get());
      }
    };
    template<typename T> T throwing_qobject_cast (QObject* from)
    {
      T x (qobject_cast<T> (from));

      if (!x)
        {
          throw std::runtime_error
            ( std::string ("throwing_qobject_cast failed from ")
//             + qPrintable(from->metaObject()->className())
//             + " to "
//             + qPrintable(typename T::staticMetaObject.className())
            );
        }

      return x;
    }
    template<typename T> T throwing_qgraphicsitem_cast (QGraphicsItem* from)
    {
      T x (qgraphicsitem_cast<T> (from));

      if (!x)
        {
          throw std::runtime_error
            ( std::string ("throwing_qgraphicsitem_cast failed from ")
//             + qPrintable(from->metaObject()->className())
//             + " to "
//             + qPrintable(typename T::staticMetaObject.className())
            );
        }

      return x;
    }

    class scoped_signal_block
    {
    private:
      QObject* _ob;
      const bool _old;
    public:
      scoped_signal_block (QObject* ob)
        : _ob (ob)
        , _old (_ob->blockSignals (true))
      {}
      ~scoped_signal_block()
      {
        _ob->blockSignals (_old);
      }
    };
  }
}

#endif

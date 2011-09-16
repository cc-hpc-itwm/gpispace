// bernd.loerwald@itwm.fraunhofer.de

#ifndef _PNETE_UTIL_HPP
#define _PNETE_UTIL_HPP 1

#include <boost/shared_ptr.hpp>

#include <QWidget>
#include <QObject>

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
  }
}

#endif

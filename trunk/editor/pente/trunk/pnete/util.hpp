// bernd.loerwald@itwm.fraunhofer.de

#ifndef _UTIL_HPP
#define _UTIL_HPP 1

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
  }
}

#endif

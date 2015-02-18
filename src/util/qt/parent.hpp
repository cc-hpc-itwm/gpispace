// bernd.loerwald@itwm.fraunhofer.de

#pragma once

#include <QWidget>
#include <QObject>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      template<typename T>
        inline T* first_parent_being_a (QWidget* widget)
      {
        while (widget && !qobject_cast<T*> (widget))
        {
          widget = widget->parentWidget();
        }
        return qobject_cast<T*> (widget);
      }

      template<typename T>
        inline T* first_parent_being_a (QObject* object)
      {
        while (object && !qobject_cast<T*> (object))
        {
          object = object->parent();
        }
        return qobject_cast<T*> (object);
      }
    }
  }
}

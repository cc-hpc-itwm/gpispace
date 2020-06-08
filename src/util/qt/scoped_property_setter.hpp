#pragma once

#include <QObject>
#include <QVariant>
#include <QString>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      class scoped_property_setter
      {
      private:
        QObject* _object;
        const QByteArray _name;
        const QVariant _old_value;

      public:
        scoped_property_setter ( QObject* object
                               , const QString& name
                               , const QVariant& new_value
                               )
          : _object (object)
          , _name (name.toAscii())
          , _old_value (_object->property (_name.data()))
        {
          _object->setProperty (_name.data(), new_value);
        }
        ~scoped_property_setter()
        {
          _object->setProperty (_name.data(), _old_value);
        }
      };
    }
  }
}

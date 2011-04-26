#include "Port.hpp"

namespace fhg
{
  namespace pnete
  {
    namespace data
    {      
      Port::Port(const QString& name, const QString& type)
      : _name(name),
      _type(type)
      {
      }
      
      const QString& Port::name() const
      {
        return _name;
      }
      
      const QString& Port::type() const
      {
        return _type;
      }
      
      QDataStream& operator<<(QDataStream& stream, const Port& port)
      {
        stream << port.name() << port.type();
        return stream;
      }
      QDataStream& operator>>(QDataStream& stream, Port& port)
      {
        QString name, type;
        stream >> name >> type;
        port = Port(name, type);
        return stream;
      }
    }
  }
}

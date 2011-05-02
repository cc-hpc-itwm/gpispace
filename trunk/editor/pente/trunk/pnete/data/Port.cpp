#include "Port.hpp"

namespace fhg
{
  namespace pnete
  {
    namespace data
    {      
      Port::Port(const QString& name, const QString& type, bool notConnectable)
      : _name(name),
      _type(type),
      _notConnectable(notConnectable)
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
      
      const bool& Port::notConnectable() const
      {
        return _notConnectable;
      }
      
      QDataStream& operator<<(QDataStream& stream, const Port& port)
      {
        QChar nc = port.notConnectable() ? 1 : 0;
        stream << nc << port.name() << port.type();
        return stream;
      }
      QDataStream& operator>>(QDataStream& stream, Port& port)
      {
        QString name, type;
        QChar notConnectable;
        stream >> notConnectable >> name >> type;
        port = Port(name, type, notConnectable != 0);
        return stream;
      }
    }
  }
}

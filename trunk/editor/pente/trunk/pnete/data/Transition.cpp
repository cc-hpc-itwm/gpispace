#include "Transition.hpp"
#include "Port.hpp"
#include "helper/XMLQuery.hpp"

#include <QStringList>

namespace fhg
{
  namespace pnete
  {
    namespace data
    {      
      Transition::Transition(const QString& path)
      {
        _name = path.mid(path.lastIndexOf("/") + 1).remove(".xml");
        
        helper::XMLQuery query(path);
        query.exec("/defun/@name", &_name);
        
        QStringList inPorts;
        query.exec("/defun/in/@name", &inPorts);
        foreach(QString port, inPorts)
        {
          QString type;
          query.exec("/defun/in[@name=\"" + port + "\"]/@type", &type);
          _inPorts.push_back(Port(port, type));
        }
        
        QStringList outPorts;
        query.exec("/defun/out/@name", &outPorts);
        foreach(QString port, outPorts)
        {
          QString type;
          query.exec("/defun/out[@name=\"" + port + "\"]/@type", &type);
          _outPorts.push_back(Port(port, type));
        }
      }
      
      Transition::Transition(const QString& name, const QList<Port>& inPorts, const QList<Port>& outPorts)
      : _name(name),
      _inPorts(inPorts),
      _outPorts(outPorts)
      {
      }
      
      Transition::Transition()
      : _name("__DUMMY__")
      {
      }
      
      const QString& Transition::name() const
      {
        return _name;
      }
      
      const QList<Port>& Transition::inPorts() const
      {
        return _inPorts;
      }
      
      const QList<Port>& Transition::outPorts() const
      {
        return _outPorts;
      }
      
      QDataStream& operator<<(QDataStream& stream, const Transition& trans)
      {
        stream << trans.name() << trans.inPorts() << trans.outPorts();
        return stream;
      }
      QDataStream& operator>>(QDataStream& stream, Transition& trans)
      {
        QString name;
        QList<Port> inPorts, outPorts;
        stream >> name >> inPorts >> outPorts;
        trans = Transition(name, inPorts, outPorts);
        return stream;
      }
    }
  }
}

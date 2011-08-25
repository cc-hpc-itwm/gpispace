#include "Transition.hpp"
#include "Port.hpp"
#include "helper/XMLQuery.hpp"

#include <QStringList>
#include <QDataStream>
#include <QXmlStreamWriter>
#include <QDebug>

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      Transition::Transition(const QString& path)
      : _path(path)
      {
                                                                                // hardcoded constant
        _name = path.mid(path.lastIndexOf("/") + 1).remove(".xml");

        helper::XMLQuery query(path);
        query.exec("/defun/@name", &_name);

        QStringList inPorts;
        query.exec("/defun/in/@name", &inPorts);
        foreach(QString port, inPorts)
        {
          QString type;
          query.exec("/defun/in[@name=\"" + port + "\"]/@type", &type);
          QStringList properties;
          query.exec("/defun/in[@name=\"" + port + "\"]/properties/property/@key", &properties);
          bool propertyPort = false;
          foreach(QString prop, properties)
          {
            propertyPort = propertyPort || prop == QString("cant_connect");
          }
          _inPorts.push_back(Port(port, type, propertyPort));
        }

        QStringList outPorts;
        query.exec("/defun/out/@name", &outPorts);
        foreach(QString port, outPorts)
        {
          QString type;
          query.exec("/defun/out[@name=\"" + port + "\"]/@type", &type);
          _outPorts.push_back(Port(port, type, false));
        }
      }

      Transition::Transition(const QString& name, const QList<Port>& inPorts, const QList<Port>& outPorts, const QString& path)
      : _path(path),
      _name(name),
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

      const QString& Transition::path() const
      {
        return _path;
      }

      const QList<Port>& Transition::inPorts() const
      {
        return _inPorts;
      }

      const QList<Port>& Transition::outPorts() const
      {
        return _outPorts;
      }

      void Transition::toXML(QXmlStreamWriter& w, const QString& transname) const
      {
      }

      QDataStream& operator<<(QDataStream& stream, const Transition& trans)
      {
        stream << trans.path() << trans.name() << trans.inPorts() << trans.outPorts();
        return stream;
      }
      QDataStream& operator>>(QDataStream& stream, Transition& trans)
      {
        QString name, path;
        QList<Port> inPorts, outPorts;
        stream >> path >> name >> inPorts >> outPorts;
        trans = Transition(name, inPorts, outPorts, path);
        return stream;
      }
    }
  }
}

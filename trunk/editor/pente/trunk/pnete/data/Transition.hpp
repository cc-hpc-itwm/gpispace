#ifndef DATATRANSITION_HPP
#define DATATRANSITION_HPP 1

#include <QString>
#include <QList>

class QDataStream;
class QXmlStreamWriter;

// for QList<Port>s. Use pointers? Who deletes those?
#include "Port.hpp"

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      class Transition
      {
        public:
          Transition();
          Transition(const QString& path);
          Transition(const QString& name, const QList<Port>& inPorts, const QList<Port>& outPorts, const QString& path = QString());

          const QString& name() const;
          const QList<Port>& inPorts() const;
          const QList<Port>& outPorts() const;
          const QString& path() const;

          void toXML(QXmlStreamWriter& streamWriter, const QString& transname = QString()) const;
          
        private:
          QString _path;
          QString _name;
          QList<Port> _inPorts;
          QList<Port> _outPorts;
      };
      QDataStream& operator<<(QDataStream& stream, const Transition& trans);
      QDataStream& operator>>(QDataStream& stream, Transition& trans);
    }
  }
}

#endif

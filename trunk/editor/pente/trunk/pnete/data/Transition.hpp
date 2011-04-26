#ifndef DATATRANSITION_HPP
#define DATATRANSITION_HPP 1

#include <QString>
#include <QList>
#include <QDataStream>

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      class Port;
      class Transition
      {
        public:
          Transition();
          Transition(const QString& path);
          Transition(const QString& name, const QList<Port>& inPorts, const QList<Port>& outPorts);
          
          const QString& name() const;
          const QList<Port>& inPorts() const;
          const QList<Port>& outPorts() const;
          
        private:
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

#include "DataGraphConversion.hpp"

#include "ui/GraphPort.hpp"
#include "ui/GraphConnection.hpp"
#include "ui/GraphTransition.hpp"

#include "data/Port.hpp"
#include "data/Connection.hpp"
#include "data/Transition.hpp"

namespace fhg
{
  namespace pnete
  {
    namespace helper
    {
      data::Transition* DataGraphConversion::transitionFromGraph(const graph::Transition& transition)
      {
        QList<data::Port> inPorts;
        QList<data::Port> outPorts;
        foreach(QGraphicsItem* child, transition.childItems())
        {
          graph::Port* port = qgraphicsitem_cast<graph::Port*>(child);
          if(port)
          {
            data::Port* dataPortPtr = portFromGraph(*port);
            data::Port dataPort = *dataPortPtr;
            delete dataPortPtr;

            switch(port->direction())
            {
              case graph::Port::IN:
                inPorts.push_back(dataPort);
                break;

              case graph::Port::OUT:
                outPorts.push_back(dataPort);
                break;

              default:
              case graph::Port::ANYDIRECTION:
                // nope, not for me.
                break;
            }
          }
        }
        return new data::Transition(transition.title(), inPorts, outPorts);
      }

      data::Port* DataGraphConversion::portFromGraph(const graph::Port& port)
      {
        return new data::Port(port.title(), port.dataType());
      }

      data::Connection* DataGraphConversion::connectionFromGraph(const graph::Connection& connection)
      {
        return new data::Connection();
      }


      graph::Transition* DataGraphConversion::transitionFromData(const data::Transition& transition)
      {
        return new graph::Transition(QString(), QString(), NULL);
      }

      graph::Port* DataGraphConversion::portFromData(const data::Port& port)
      {
        return new graph::Port(NULL, graph::ConnectableItem::ANYDIRECTION, QString(), QString());
      }

      graph::Connection* DataGraphConversion::connectionFromData(const data::Connection& connection)
      {
        return new graph::Connection();
      }

    }
  }
}

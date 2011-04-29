#include "GraphTraverser.hpp"
#include "DataGraphConversion.hpp"
#include "TraverserReceiver.hpp"

#include "graph/Scene.hpp"
#include "graph/Port.hpp"
#include "graph/Connection.hpp"
#include "graph/Transition.hpp"

#include "data/Port.hpp"
#include "data/Connection.hpp"
#include "data/Transition.hpp"

#include <QXmlStreamWriter>
#include <QDebug>

namespace fhg
{
  namespace pnete
  {
    namespace helper
    {      
      GraphTraverser::GraphTraverser(const graph::Scene* scene)
      : _scene(scene)
      {
      }
      
      QString makeValidName(const QString& name)
      {
        QString ret = name;
        return ret.replace(" ", "____");
      }
      
      void GraphTraverser::traverse(TraverserReceiver* dataReceiver, const QString& fileName) const
      {
        QString xml;
        QXmlStreamWriter w(&xml);
        w.setAutoFormatting(true);
          
        QList<graph::Port*> openPorts;
        
        w.writeStartElement("defun");
        w.writeAttribute("name", makeValidName(fileName));
        
        w.writeStartElement("net");
        
        foreach(QGraphicsItem* item, _scene->items())
        {
          graph::Connection* connection = qgraphicsitem_cast<graph::Connection*>(item);
          graph::Transition* transition = qgraphicsitem_cast<graph::Transition*>(item);
          
          if(connection)
          {
            if(connection->start() && connection->end())
            {
              w.writeStartElement("place");
              w.writeAttribute("name", makeValidName(QString("place_%1").arg((long)connection, 0, 16)));
              w.writeAttribute("type", qgraphicsitem_cast<const graph::Port*>(connection->start())->dataType());
              w.writeEndElement();
            }
          }
          else if(transition)
          {
            w.writeStartElement("transition");
            
            // write the transition with its ports
            if(transition->producedFrom().path() != QString())
            {
              w.writeStartElement("include-function");
              w.writeAttribute("href", transition->producedFrom().path());
              w.writeEndElement();
            }
            else
            {
              w.writeStartElement("defun");
              if(transition->title() != QString())
              {
                w.writeAttribute("name", makeValidName(transition->title()));
              }
              
              foreach(QGraphicsItem* child, transition->childItems())
              {
                graph::Port* port = qgraphicsitem_cast<graph::Port*>(child);
                if(port)
                {
                  w.writeStartElement(port->direction() == graph::Port::IN ? "in" : "out");
                  w.writeAttribute("name", makeValidName(port->title()));
                  w.writeAttribute("type", port->dataType());
                  w.writeEndElement();
                }
              }
              
              w.writeEndElement();
            }
            
            // write all connections.
            foreach(QGraphicsItem* child, transition->childItems())
            {
              graph::Port* port = qgraphicsitem_cast<graph::Port*>(child);
              if(port)
              {
                if(port->connection())
                {
                  w.writeStartElement(port->direction() == graph::Port::IN ? "connect-in" : "connect-out");
                  w.writeAttribute("port", makeValidName(port->title()));
                  w.writeAttribute("place", makeValidName(QString("place_%1").arg((long)port->connection(), 0, 16)));
                  w.writeEndElement();
                }
                else
                {
                  w.writeStartElement(port->direction() == graph::Port::IN ? "connect-in" : "connect-out");
                  w.writeAttribute("port", makeValidName(port->title()));
                  w.writeAttribute("place", makeValidName(QString("place_%1_%2").arg((long)port->parentItem(), 0, 16).arg(port->title())));
                  w.writeEndElement();
                  openPorts.push_back(port);
                }
              }
            }
            w.writeEndElement();
          }
        }
        
        foreach(graph::Port* port, openPorts)
        {
          w.writeStartElement("place");
          w.writeAttribute("name", makeValidName(QString("place_%1_%2").arg((long)port->parentItem(), 0, 16).arg(port->title())));
          w.writeAttribute("type", port->dataType());
          w.writeEndElement();
        }
        
        w.writeEndElement();
        
        foreach(graph::Port* port, openPorts)
        {
          w.writeStartElement(port->direction() == graph::Port::IN ? "in" : "out");
          w.writeAttribute("name", makeValidName(QString("%2_%1").arg((long)port->parentItem(), 0, 16).arg(port->title())));
          w.writeAttribute("type", port->dataType());
          w.writeAttribute("place", makeValidName(QString("place_%1_%2").arg((long)port->parentItem(), 0, 16).arg(port->title())));
          w.writeEndElement();
        }
        
        w.writeEndElement();
        
        /*
          w.writeStartElement(port->direction() == graph::Port::IN ? "in" : "out");
          w.writeAttribute("port", port->title());
          w.writeAttribute("place", QString("place_%1_%2").arg((long)port->parentItem(), 0, 16).arg(port->title()));
          w.writeEndElement();
        
          <in name="desc" type="string" place="desc"/>*/
        
        qDebug() << xml;
      }
    }
  }
}

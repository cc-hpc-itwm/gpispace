#include "TraverserReceiver.hpp"

#include "data/Transition.hpp"
#include "data/Connection.hpp"
#include "data/Port.hpp"
#include "graph/Scene.hpp"

#include <QDebug>
#include <QXmlStreamWriter>

namespace fhg
{
  namespace pnete
  {
    namespace helper
    {
      TraverserReceiver::TraverserReceiver()
      {
      } 
      TraverserReceiver::~TraverserReceiver()
      {
      }
          
      void TraverserReceiver::transitionFound(const data::Transition& transition)
      {

      }
      
      void TraverserReceiver::connectionFound(const data::Connection& connection)
      {
      }
    }
  }
}

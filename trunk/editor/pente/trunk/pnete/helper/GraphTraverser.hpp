#ifndef HELPERGRAPHTRAVERSER_HPP
#define HELPERGRAPHTRAVERSER_HPP 1

#include <QString>

namespace fhg
{
  namespace pnete
  {
    namespace graph
    {
      class Scene;
    }
    namespace helper
    {
      class TraverserReceiver;
      class GraphTraverser
      {
        public:
          GraphTraverser(const graph::Scene* scene);
          
          void traverse(TraverserReceiver* dataReceiver, const QString& fileName = "myNet") const;
          
        private:
          const graph::Scene* _scene;
      };
    }
  }
}

#endif

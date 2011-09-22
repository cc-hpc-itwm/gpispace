#include "taskscene.hpp"

namespace fhg
{
  namespace taskview
  {
    TaskScene::TaskScene (QObject *parent)
      : QGraphicsScene(parent)
    {
      setSceneRect(0,0,0,0);
      setItemIndexMethod(QGraphicsScene::NoIndex);
    }
  }
}

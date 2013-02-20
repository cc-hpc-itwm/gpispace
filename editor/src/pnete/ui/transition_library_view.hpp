// mirko.rahn@itwm.fraunhofer.de

#ifndef _FHG_PNETE_UI_TRANSITION_LIBRARY_VIEW_HPP
#define _FHG_PNETE_UI_TRANSITION_LIBRARY_VIEW_HPP 1

#include <QTreeView>

class QWidget;
class QResizeEvent;

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      class transition_library_view : public QTreeView
      {
      public:
        explicit transition_library_view (int width1, int margin, QWidget* = 0);

      protected:
        virtual void resizeEvent (QResizeEvent*);

      private:
        int _width1;
        int _margin;
      };
    }
  }
}

#endif

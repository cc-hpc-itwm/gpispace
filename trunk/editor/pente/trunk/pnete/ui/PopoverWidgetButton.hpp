#ifndef UIPOPOVERWIDGETBUTTON_HPP
#define UIPOPOVERWIDGETBUTTON_HPP 1

#include <QGraphicsProxyWidget>

class QGraphicsItem;
class QToolButton;

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      class PopoverWidgetButton : public QGraphicsProxyWidget
      {
        Q_OBJECT
        public:
          PopoverWidgetButton(QGraphicsItem* parent = NULL);

        public slots:
          void openPopover();

        private:
          QToolButton* _cogwheelButton;
          QGraphicsProxyWidget* _popup;
      };
    }
  }
}

#endif

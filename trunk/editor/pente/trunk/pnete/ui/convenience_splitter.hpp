// bernd.loerwald@itwm.fraunhofer.de

#ifndef UI_CONVENIENCE_SPLITTER_HPP
#define UI_CONVENIENCE_SPLITTER_HPP 1

#include <QObject>
#include <QSplitter>

class QWidget;

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      class view_manager;
      class splittable_tab_widget;

      class convenience_splitter : public QSplitter
      {
        Q_OBJECT

        protected:
          view_manager* _view_manager;

        public:
          convenience_splitter ( view_manager* view_manager_
                               , const Qt::Orientation& orientation
                               , QWidget* parent = NULL
                               );
          convenience_splitter ( view_manager* view_manager_
                               , QWidget* parent = NULL
                               );

          void add (splittable_tab_widget* widget);

        public slots:
          void split (splittable_tab_widget*, const Qt::Orientation&);
          void remove (splittable_tab_widget*);
      };
    }
  }
}

#endif

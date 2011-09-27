// mirko.rahn@itwm.fraunhofer.de


#include <pnete/ui/ports_list_widget.hpp>

#include <QWidget>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QSplitter>

#include <pnete/ui/port_list_widget.hpp>

#define TEST

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      static QGroupBox* port_list_box
        ( const QString & label
        , data::proxy::xml_type::ports_type & ports
        , const QStringList& list_types
        )
      {
        QGroupBox* group_box (new QGroupBox (label));
        QVBoxLayout* vbox (new QVBoxLayout (group_box));
        vbox->addWidget (new port_list_widget (ports, list_types));
        group_box->setLayout (vbox);
        return group_box;
      }

      ports_list_widget::ports_list_widget
        ( data::proxy::xml_type::ports_type& in
        , data::proxy::xml_type::ports_type& out
        , QWidget* parent
        )
          : QWidget (parent)
          , _in (in)
          , _out (out)
      {
        QSplitter* splitter (new QSplitter(Qt::Vertical));

#ifdef TEST
        init_test_data(_in);
        init_test_data(_out);
#endif //TEST

        splitter->addWidget(port_list_box (tr ("in"),  _in, list_combo_types()));
        splitter->addWidget(port_list_box (tr ("out"), _out, list_combo_types()));

        QVBoxLayout* vbox (new QVBoxLayout());
        vbox->addWidget (splitter);
        vbox->setContentsMargins (0, 0, 0, 0);
        setLayout(vbox);
      }

      void ports_list_widget::init_test_data(data::proxy::xml_type::ports_type& ports)
      {
		  fhg::util::maybe<std::basic_string<char> > mbe;

		  for( int k=0; k<5; k++)
		  {
			  QString qstrName = QString("name %1").arg(k);
			  QString qstrType = QString("type %1").arg(k);

			  xml::parse::type::port_type port(qstrName.toStdString(), qstrType.toStdString(), mbe);
			  ports.push_back(port);
		  }
      }

      QStringList ports_list_widget::list_combo_types()
	  {
		  QStringList qstrList;
		  for( int k=0; k<5; k++)
			  qstrList<<QString("type %1").arg(k);

		  return qstrList;
	  }
    }
  }
}


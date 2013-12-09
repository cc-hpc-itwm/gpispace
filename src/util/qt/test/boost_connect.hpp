// bernd.loerwald@itwm.fraunhofer.de

#include <QObject>
#include <QMetaType>

struct custom_type
{
  int dummy;
  custom_type (int d) : dummy (d) { }
  custom_type() : dummy (-1) { }
};

Q_DECLARE_METATYPE (custom_type);

class boost_connect_fixture : public QObject
{
  Q_OBJECT;

public:
  boost_connect_fixture()
    : _called (false)
    , _ival (0)
  { }

  void called()
  {
    _called = true;
  }

  void function (int val);

signals:
  void signal1();
  void signal2 (int);
  void signal3 (const QByteArray&);
  void signal4 (unsigned int, unsigned int);
  void signal5 (const custom_type&);

protected:
  bool _called;
  int _ival;
};

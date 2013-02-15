// bernd.loerwald@itwm.fraunhofer.de

#include <QObject>

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

protected:
  bool _called;
  int _ival;
};

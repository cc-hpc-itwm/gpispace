// bernd.loerwald@itwm.fraunhofer.de


#include <QDebug>

#include <QtCore/QMetaMethod>
#include <QtCore/QMetaObject>

#include <fhg/assert.hpp>

namespace
{
  const QString q4pugss_value (const char *type, void *argv)
  {
    QVariant v (QMetaType::type (type), argv);

    if (v.type() != QVariant::Invalid)
    {
      return QString("%1 (%2)").arg (type).arg (v.toString());
    }

    return QString("%1 (...)").arg (type);
  }

  int indent (0);

  void callback_begin (QObject *caller, int method_index, void **argv)
  {
    const QMetaObject *mo = caller->metaObject();

    fhg_assert (mo, "caller must have meta object");
    fhg_assert (method_index < mo->methodCount(), "method must be in method table");

    QMetaMethod m = mo->method (method_index);

    static QString methodType[] = { " ", ">", "<" };

    QString string;
    string += QString::number (size_t (caller->thread()));
    string += QString (indent * 2, ' ');
    string += " ";
    string += methodType[int (m.methodType())];
    string += " ";
    string += mo->className();
    if (!caller->objectName().isNull())
    {
      string += " \"";
      string += caller->objectName();
      string += "\"";
    }
    string += " ";
    string += QString (m.signature()).section ('(', 0, 0);

    const QList<QByteArray> pNames (m.parameterNames());
    const QList<QByteArray> pTypes (m.parameterTypes());

    if (!pTypes.empty())
    {
      string += " ";
    }
    string += "(";

    for (int i = 0; i < pNames.count(); ++i)
    {
      string += QString("%1 := %2")
        .arg(QString(pNames.at(i)))
        .arg(q4pugss_value(pTypes.at(i), argv[i + 1]));

      if (i != pNames.count() - 1)
      {
        string += ", ";
      }
    }

    string += ")\n";

    std::cerr << string.toStdString();

    ++indent;
  }

  void callback_end (QObject *, int)
  {
    --indent;
  }
}

struct QSignalSpyCallbackSet
{
  typedef void (*BeginCallback)(QObject *caller, int method_index, void **argv);
  typedef void (*EndCallback)(QObject *caller, int method_index);

  BeginCallback signal_begin_callback;
  BeginCallback slot_begin_callback;
  EndCallback signal_end_callback;
  EndCallback slot_end_callback;
};

void Q_CORE_EXPORT qt_register_signal_spy_callbacks (const QSignalSpyCallbackSet &callback_set);

extern QSignalSpyCallbackSet Q_CORE_EXPORT qt_signal_spy_callback_set;

void q4pugss_registerCallBacks()
{
  QSignalSpyCallbackSet cb;
  cb.signal_begin_callback = callback_begin;
  cb.signal_end_callback = callback_end;
  cb.slot_begin_callback = callback_begin;
  cb.slot_end_callback = callback_end;

  qt_register_signal_spy_callbacks (cb);
}

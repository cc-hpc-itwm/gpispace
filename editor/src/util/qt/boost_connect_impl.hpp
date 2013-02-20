// bernd.loerwald@itwm.fraunhofer.de

/* Based on http://gitorious.org/qtboostintegration/qtboostintegration/
 * Copyright 2010  Benjamin K. Stuhl <bks24@cornell.edu>
 *
 * Permission to use, copy, modify, and distribute this software
 * and its documentation for any purpose and without fee is hereby
 * granted, provided that the above copyright notice appear in all
 * copies and that both that the copyright notice and this
 * permission notice and warranty disclaimer appear in supporting
 * documentation, and that the name of the author not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * The author disclaim all warranties with regard to this
 * software, including all implied warranties of merchantability
 * and fitness.  In no event shall the author be liable for any
 * special, indirect or consequential damages or any damages
 * whatsoever resulting from loss of use, data or profits, whether
 * in an action of contract, negligence or other tortious action,
 * arising out of or in connection with the use or performance of
 * this software.
 */

#ifndef FHG_UTIL_QT_BOOST_CONNECT_IMPL_HPP
#define FHG_UTIL_QT_BOOST_CONNECT_IMPL_HPP

#include <QtCore/QObject>
#include <QtCore/QList>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      namespace boost_connect_detail
      {
        class abstract_connection_adapter;

        class binding_object : public QObject
        {
          //! \note no Q_OBJECT, since we don't want moc to run

        public:
          explicit binding_object (QObject *parent = NULL);
          virtual ~binding_object();

          bool bind ( QObject* sender
                    , const char* signal
                    , QObject* receiver
                    , abstract_connection_adapter* adapter
                    , const QByteArray& fake_signature
                    , Qt::ConnectionType connType
                    );

          bool unbind (QObject *sender, const char *signal, QObject *receiver);

          int binding_offset() const
          {
            return metaObject()->methodOffset() + metaObject()->methodCount();
          }

        public slots:
          void receiver_destroyed();
          void sender_destroyed();

          // core QObject stuff: we implement this ourselves rather than
          // via moc, since qt_metacall() is the core of the binding
          static const QMetaObject staticMetaObject;
          virtual const QMetaObject *metaObject() const;
          virtual void *qt_metacast (const char *);
          virtual int qt_metacall (QMetaObject::Call, int, void **argv);

        private:
          struct binding
          {
            QObject* sender;
            QObject* receiver;
            abstract_connection_adapter* adapter;
            int signal_index;

            binding (QObject* s, int i, QObject* r, abstract_connection_adapter* a)
              : sender (s)
              , receiver (r)
              , adapter (a)
              , signal_index (i)
            { }
          };

          QList<binding> _bindings;
        };
      }
    }
  }
}

#endif

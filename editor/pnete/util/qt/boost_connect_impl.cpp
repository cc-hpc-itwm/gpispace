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

#include <util/qt/boost_connect_impl.hpp>
#include <util/qt/boost_connect.hpp>

#include <QtCore/QMetaMethod>
#include <QtCore/QThread>
#include <QtCore/QVector>
#include <QtCore/QThreadStorage>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      namespace
      {
        QThreadStorage<boost_connect_detail::binding_object*> binding_objects;
      }

      bool boost_disconnect
        (QObject *sender, const char *signal, QObject *receiver)
      {
        boost_connect_detail::binding_object* bindingObj
          (binding_objects.localData());
        if (!bindingObj)
        {
          return false;
        }

        return bindingObj->unbind (sender, signal, receiver);
      }

      namespace boost_connect_detail
      {
        bool connect_impl
          ( QObject *sender
          , const char *signal
          , QObject *receiver
          , abstract_connection_adapter *adapter
          , const QByteArray& fake_signature
          , Qt::ConnectionType connType
          )
        {
          binding_object *bindingObj (binding_objects.localData());
          if (!bindingObj)
          {
            bindingObj = new binding_object (QThread::currentThread());
            binding_objects.setLocalData (bindingObj);
          }

          return bindingObj->bind ( sender
                                  , signal
                                  , receiver
                                  , adapter
                                  , fake_signature
                                  , connType
                                  );
        }

        namespace
        {
          class connect_notify_object : public QObject
          {
          public:
            void call_connect_notify (const char* signal)
            {
              connectNotify (signal);
            }
            void call_disconnect_notify (const char* signal)
            {
              disconnectNotify (signal);
            }
          };
        }

        binding_object::binding_object (QObject* parent)
          : QObject(parent)
        { }

        binding_object::~binding_object()
        {
          foreach (const binding& b, _bindings)
          {
            delete b.adapter;
          }
        }

        //! \note This is a manually moc'd meta object!
        static const uint qt_meta_data_binding_object[] = {
          5,       // qt 4.8.3
          0x0,     // class name: binding_object\0
          0,  0x0, // no class info
          2,  0xE, // methods: 2, starting at 0xE in this struct
          0,  0x0, // no properties
          0,  0x0, // no enums/sets
          0,  0x0, // no constructors
          0,       // no flags
          0,       // no signals

          0x0F, // signature: receiver_destroyed()\0
          0x0E, // parameters: \0
          0x0E, // return type: \0
          0x0E, // tag: \0
          0x0A, // flags: public slot

          0x24, // signature: sender_destroyed()\0
          0x0E, // parameters: \0
          0x0E, // return type: \0
          0x0E, // tag: \0
          0x0A, // flags: public slot

          0        // eod
        };

        static const char qt_meta_stringdata_binding_object[] = {
          /* 0x00 */ "binding_object\0"
          /* 0x0F */ "receiver_destroyed()\0"
          /* 0x24 */ "sender_destroyed()\0"
        };

        const QMetaObject binding_object::staticMetaObject = {
          { &QObject::staticMetaObject
          , qt_meta_stringdata_binding_object
          , qt_meta_data_binding_object
          , 0
          }};

        const QMetaObject* binding_object::metaObject() const
        {
          return ( QObject::d_ptr->metaObject
                 ? QObject::d_ptr->metaObject
                 : &staticMetaObject
                 );
        }

        void* binding_object::qt_metacast (const char* _clname)
        {
          if (!_clname)
          {
            return 0;
          }
          if (!strcmp (_clname, qt_meta_stringdata_binding_object))
          {
            return static_cast<void*> (this);
          }
          return QObject::qt_metacast (_clname);
        }

        int binding_object::qt_metacall
          (QMetaObject::Call _c, int _id, void** _a)
        {
          // handle QObject base metacalls
          _id = QObject::qt_metacall(_c, _id, _a);
          if (_id < 0)
          {
            return _id;
          }

          // handle our one named slot
          if (_id == 0)
          {
            receiver_destroyed();
            return -1;
          }
          else if (_id == 1)
          {
            sender_destroyed();
            return -1;
          }

          // it must be a binding call, offset it by our methodCount()
          // and try to handle it
          _id -= metaObject()->methodCount();
          if ( _c == QMetaObject::InvokeMetaMethod
             && _id < _bindings.size() && _bindings[_id].adapter)
          {
            // if we have a binding for this index, call it
            _bindings[_id].adapter->invoke(_a);
            _id -= _bindings.size();
          }
          return _id;
        }

        bool binding_object::bind ( QObject* sender
                                  , const char* signal
                                  , QObject* receiver
                                  , abstract_connection_adapter* adapter
                                  , const QByteArray& fake_signature
                                  , Qt::ConnectionType connType
                                  )
        {
          if (!sender || !signal || !adapter)
          {
            return false;
          }

          QByteArray signal_signature
            (sender->metaObject()->normalizedSignature (signal));
          const int signal_index
            (sender->metaObject()->indexOfSignal (signal_signature.data() + 1));
          if (signal_index < 0)
          {
            qWarning ( "qtBoostConnect: no match for signal '%s'"
                     , signal_signature.data() + 1
                     );
            return false;
          }

          Q_ASSERT_X ( !receiver || receiver->thread() == thread()
                     , "qtBoostConnect"
                     , "Receiving QObject on different thread."
                     );

          if (!QMetaObject::checkConnectArgs (signal_signature, fake_signature))
          {
            return false;
          }

          // wire up a connection from the binding object to the sender
          if ( !QMetaObject::connect ( sender
                                     , signal_index
                                     , this
                                     , _bindings.size() + binding_offset()
                                     , connType
                                     )
             )
          {
            return false;
          }

          bool already_knows_this_receiver = false;
          bool already_knows_this_sender = false;

          for (int i (0); i < _bindings.size(); ++i)
          {
            if (_bindings[i].receiver == receiver)
            {
              already_knows_this_receiver = true;
            }
            if (_bindings[i].sender == sender)
            {
              already_knows_this_sender = true;
            }
          }

          // store the binding
          _bindings.append (binding (sender, signal_index, receiver, adapter));

          // and make sure that we will delete it if the receiver goes away
          if (!already_knows_this_receiver && receiver)
          {
            QObject::connect
              (receiver, SIGNAL(destroyed()), this, SLOT(receiver_destroyed()));
          }
          if (!already_knows_this_sender && sender)
          {
            QObject::connect
              (sender, SIGNAL(destroyed()), this, SLOT(sender_destroyed()));
          }

          static_cast<connect_notify_object*> (sender)->
            call_connect_notify (signal);

          return true;
        }

        bool binding_object::unbind
          (QObject* sender, const char* signal, QObject* receiver)
        {
          int signal_index = -1;

          if (signal)
          {
            QByteArray signal_signature
              (sender->metaObject()->normalizedSignature (signal));
            signal_index =
              sender->metaObject()->indexOfSignal (signal_signature.data() + 1);
            if (signal_index < 0)
            {
              return false;
            }
          }

          bool found = false;
          QVector<int> to_be_removed;
          for (int i = 0; i < m_bindings.size(); i++)
          {
            const Binding& b = m_bindings.at (i);
            if ( (b.signalIndex == signalIndex || signalIndex == -1)
               && (b.sender == sender || !sender)
               && (b.receiver == receiver || !receiver)
               )
            {
              QMetaObject::disconnect
                (b.sender, b.signalIndex, b.receiver, i + bindingOffset());
              QByteArray sigName
                (b.sender->metaObject()->method (b.signalIndex).signature());
              sigName.prepend ('2');
              static_cast<connect_notify_object*> (b.sender)->
                call_disconnect_notify (sigName.constData());
              delete b.adapter;
              to_be_removed.push_back (i);
              found = true;
            }
          }
          foreach (const int i, to_be_removed)
          {
            _bindings.removeAt (i);
          }

          return found;
        }

        // when any object for which we are holding a binding is destroyed,
        // remove all of its bindings
        void binding_object::receiver_destroyed()
        {
          QObject* obj = sender();

          if (obj)
          {
            unbind (NULL, NULL, obj);
          }
        }
        void binding_object::sender_destroyed()
        {
          QObject* obj = sender();

          if (obj)
          {
            unbind (obj, NULL, NULL);
          }
        }
      }
    }
  }
}

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
          , int nrArguments
          , int argumentTypeList[]
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
                                  , nrArguments
                                  , argumentTypeList
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
          foreach (const Binding& b, m_bindings)
          {
            delete b.adapter;
          }
        }

        //! \note This is a manually moc'd meta object!
        static const uint qt_meta_data_binding_object[] = {
          5,       // qt 4.8.3
          0x0,     // class name: binding_object\0
          0,  0x0, // no class info
          1,  0xE, // methods: 1, starting at 0xE in this struct
          0,  0x0, // no properties
          0,  0x0, // no enums/sets
          0,  0x0, // no constructors
          0,       // no flags
          0,       // no signals

          0x0F, // signature: receiver_destroyed()\0
          0x2F, // parameters: \0
          0x2F, // return type: \0
          0x2F, // tag: \0
          0x0A, // flags: public slot

          0        // eod
        };

        static const char qt_meta_stringdata_binding_object[] = {
          /* 0x00 */ "binding_object\0"
          /* 0x0F */ "receiver_destroyed()\0"
          /* 0x24 */ "\0"
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

          // it must be a binding call, offset it by our methodCount()
          // and try to handle it
          _id -= metaObject()->methodCount();
          if ( _c == QMetaObject::InvokeMetaMethod
             && _id < m_bindings.size() && m_bindings[_id].adapter)
          {
            // if we have a binding for this index, call it
            m_bindings[_id].adapter->invoke(_a);
            _id -= m_bindings.size();
          }
          return _id;
        }

        bool binding_object::bind ( QObject* sender
                                  , const char* signal
                                  , QObject* receiver
                                  , abstract_connection_adapter* adapter
                                  , int nrArguments
                                  , int argumentList[]
                                  , Qt::ConnectionType connType
                                  )
        {
          if (!sender || !signal || !adapter)
          {
            return false;
          }

          QByteArray signalSignature
            (sender->metaObject()->normalizedSignature (signal));
          const int signalIndex
            (sender->metaObject()->indexOfSignal (signalSignature.data() + 1));
          if (signalIndex < 0)
          {
            qWarning ( "qtBoostConnect: no match for signal '%s'"
                     , signalSignature.data() + 1
                     );
            return false;
          }

          Q_ASSERT_X ( !receiver || receiver->thread() == thread()
                     , "qtBoostConnect"
                     , "Receiving QObject on different thread."
                     );

          QByteArray adapterSignature
            (buildAdapterSignature (nrArguments, argumentList));

          if (!QMetaObject::checkConnectArgs(signalSignature, adapterSignature))
          {
            return false;
          }

          // locate a free slot
          int slotIndex;
          bool alreadyKnowAboutThisReceiver = false;
          for (slotIndex = 0; slotIndex < m_bindings.size(); slotIndex++)
          {
            if (m_bindings[slotIndex].receiver == receiver)
            {
              alreadyKnowAboutThisReceiver = true;
            }
            if (!m_bindings[slotIndex].adapter)
            {
              break;
            }
          }

          // wire up a connection from the binding object to the sender
          if ( !QMetaObject::connect
               (sender, signalIndex, this, slotIndex + bindingOffset(), connType)
             )
          {
            return false;
          }

          // store the binding
          if (slotIndex == m_bindings.size())
          {
            m_bindings.append (Binding (sender, signalIndex, receiver, adapter));
          }
          else
          {
            m_bindings.replace
              (slotIndex, Binding (sender, signalIndex, receiver, adapter));
          }

          // and make sure that we will delete it if the receiver goes away
          // ### should we auto-delete if senders disappear, too?
          // ###  it costs memory on connections to possibly save memory later...
          if (!alreadyKnowAboutThisReceiver && receiver)
          {
            QObject::connect
              (receiver, SIGNAL(destroyed()), this, SLOT(receiver_destroyed()));
          }

          static_cast<connect_notify_object*> (sender)->
            call_connect_notify (signal);
          return true;
        }

        bool binding_object::unbind
          (QObject* sender, const char* signal, QObject* receiver)
        {
          int signalIndex = -1;

          if (signal)
          {
            QByteArray signalSignature
              (sender->metaObject()->normalizedSignature (signal));
            signalIndex =
              sender->metaObject()->indexOfSignal (signalSignature.data() + 1);
            if (signalIndex < 0)
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
               && b.receiver == receiver
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
            m_bindings.removeAt (i);
          }

          return found;
        }

        QByteArray binding_object::buildAdapterSignature
          (int nrArguments, int argumentMetaTypeList[])
        {
          QByteArray sig ("lambda(");
          for (int i = 0; i < nrArguments; i++)
          {
            sig += QMetaType::typeName (argumentMetaTypeList[i]);
            if (i != nrArguments-1)
            {
              sig += ",";
            }
          }
          sig += ")";
          return sig;
        }

        void binding_object::receiver_destroyed()
        {
          // when any object for which we are holding a binding is destroyed,
          // remove all of its bindings
          QObject* receiver = sender();

          if (receiver)
          {
            unbind (0, 0, receiver);
          }
        }
      }
    }
  }
}

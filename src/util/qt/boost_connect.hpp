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

#pragma once

#include <fhg/util/make_indices.hpp>

#include <QtCore/QObject>
#include <QtCore/QMetaType>

#include <functional>
#include <type_traits>

namespace fhg
{
  namespace util
  {
    namespace qt
    {
      namespace boost_connect_detail
      {
        class abstract_connection_adapter
        {
        public:
          virtual ~abstract_connection_adapter() = default;

          virtual void invoke (void** args) = 0;
        };

        namespace
        {
          template<typename T>
          using remove_const_reference
            = typename std::remove_const<typename std::remove_reference<T>::type>::type;

          template<typename...> struct append_qMetaTypeId;
          template<> struct append_qMetaTypeId<>
          {
            static QByteArray apply (QByteArray bytearray) { return bytearray; }
          };
          template<typename U, typename... Rest>
            struct append_qMetaTypeId<U, Rest...>
          {
            static QByteArray apply (QByteArray bytearray)
            {
              bytearray.append
                (QMetaType::typeName (qMetaTypeId<remove_const_reference<U>>()));
              if (sizeof... (Rest))
              {
                bytearray.append (",");
              }
              return append_qMetaTypeId<Rest...>::apply (bytearray);
            }
          };
        }

        template<typename Signature> class connection_adapter;
        template<typename... T>
          class connection_adapter<void (T...)>
          : public abstract_connection_adapter
        {
        public:
          explicit connection_adapter (std::function<void (T...)> const& f)
            : _invoke_impl (f)
          { }

          static QByteArray build_signature()
          {
            return append_qMetaTypeId<T...>::apply (QByteArray ("fake("))
              .append (")");
          }

          virtual void invoke (void** args) override
          {
            _invoke_impl.apply (args);
          }

        private:
          template<typename> struct invoke_impl;
          template<std::size_t... Indices>
            struct invoke_impl<indices<Indices...>>
          {
            invoke_impl (std::function<void (T...)> const& function)
              : _function (function)
            {}
            void apply (void** args)
            {
              return _function ( *reinterpret_cast<remove_const_reference<T>*>
                                   (args[Indices+1])...
                               );
            }
            std::function<void (T...)> _function;
          };
          invoke_impl<fhg::util::make_indices<sizeof... (T)>> _invoke_impl;
        };
        template<>
          class connection_adapter<void()> : public abstract_connection_adapter
        {
        public:
          explicit connection_adapter (std::function<void()> const& f)
            : _function (f)
          {}
          static QByteArray build_signature()
          {
            return "fake()";
          }
          virtual void invoke (void**) override
          {
            _function();
          }
        private:
          std::function<void()> _function;
        };

        bool connect_impl ( QObject* sender
                          , const char* signal
                          , QObject* receiver
                          , abstract_connection_adapter* adapter
                          , const QByteArray& fake_signature
                          , Qt::ConnectionType connection_type
                          );
      }

      template<typename Sig>
        inline bool boost_connect ( QObject* sender
                                  , const char* signal
                                  , QObject* receiver
                                  , const std::function<Sig>& function
                                  , Qt::ConnectionType connection_type
                                  = Qt::AutoConnection
                                  )
      {
        return boost_connect_detail::connect_impl
          ( sender
          , signal
          , receiver
          , new boost_connect_detail::connection_adapter<Sig> (function)
          , boost_connect_detail::connection_adapter<Sig>::build_signature()
          , connection_type
          );
      }

      template<typename Signature>
        inline bool boost_connect ( QObject* sender
                                  , const char* signal
                                  , const std::function<Signature>& function
                                  , Qt::ConnectionType connection_type
                                  = Qt::AutoConnection
                                  )
      {
        return boost_connect (sender, signal, nullptr, function, connection_type);
      }

      bool boost_disconnect ( QObject* sender
                            , const char* signal
                            , QObject* receiver
                            );
    }
  }
}
